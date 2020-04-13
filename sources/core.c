#include <assertions.h>
#include <handlers.h>
#include <pcb.h>
#include <core.h>

// NOTE: keep this portion of code free of arch-specific code!

static void enterKernelMode(cpustate_t *self);
static void enterUserMode(cpustate_t *self);
static void setVirtualMemory(cpustate_t *self, bool v);
static void setFastInterrupts(cpustate_t *self, bool v);
static void setInterrupts(cpustate_t *self, bool v);

/**
 * Registers the handler to be executed when an exception is raised.
 * The handler will be executed in kernel mode with both interrupts and virtual
 * memory disabled.
 *
 * @param state   New state of the cpu to be loaded while handling the exception.
 * @param handler Function to handle the exception.
 */
static void registerHandler(cpustate_t *const state, void (*const handler)(void)) {
    debug_assert(NULL != state);
    debug_assert(NULL != handler);

    state_clear(state);
    *state_programCounter(state) = (memaddr) handler;
    state_setStackPointer(state, MACHINE_RAM_LIMIT);
}

/**
 * Initializes the machine registering handlers and initializing pcbs, etc...
 */
static void setup(void) {
    // register handlers
    registerHandler(MACHINE_NEW_SYSBK_AREA, handlers_sysbkHandler);
    registerHandler(MACHINE_NEW_INTERRUPT_AREA, handlers_interruptHandler);
    registerHandler(MACHINE_NEW_PRGM_TRAP_AREA, handlers_unexpectedHandler);
    registerHandler(MACHINE_NEW_TLB_MGMT_AREA, handlers_unexpectedHandler);

    // initialize pcbs
    initPcbs();
}

void core_boot(void) {
    cpustate_t state;

    core_storeState(&state);
    state_clear(&state);
    *state_programCounter(&state) = (memaddr) setup;

    core_loadState(&state);
}

/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!    ATTENTION    !!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * The code below contains arch-dependant code!
 *
 * From here on until the next comment there is the code of the
 * functions that are machine-dependent but can be implemented in the same way
 * for both uARM and uMPS.
 */

#if !(defined(TARGET_UARM) || defined(TARGET_UMPS))
#error "Unknown target architecture"
#endif

void core_storeState(cpustate_t *const out) {
    debug_assert(NULL != out);
    STST(out);
    *state_programCounter(out) = 0;
}

void core_loadState(cpustate_t *const state) {
    debug_assert(NULL != state);
    LDST(state);
}

void core_halt(void) {
    HALT();
    for(;;) {} // ensure noreturn and quiet compiler
}

void core_panic(void) {
    /// @attention NEVER call assertions here!!!!
    PANIC();
    for(;;) {} // ensure noreturn and quiet compiler
}

unsigned machine_getInterruptLine(void) {
#if defined(TARGET_UARM)
#define CAUSE_IP(il)    (1 << ((il) + 24))
#endif

    const unsigned cause = getCAUSE();

    for (unsigned i = 0; i < N_INTERRUPT_LINES; ++i) {
        if (cause & CAUSE_IP(i)) {
            return i;
        }
    }

    unreachable();
}

unsigned machine_getInterruptDevice(const unsigned il) {
    debug_assert(N_INTERRUPT_LINES > il);
    const unsigned line = *((unsigned *) CDEV_BITMAP_ADDR(il));

    for (unsigned i = 0; i < N_DEV_PER_IL; ++i) {
        if (line & (1 << i)) {
            return i;
        }
    }

    unreachable();
}

ticks_t machine_getClockResolution(void) {
    return *((ticks_t *) BUS_REG_TIME_SCALE);
}

ticks_t machine_getIntervalTimer(void) {
    return *((ticks_t *) BUS_REG_TIMER);
}

void machine_setIntervalTimer(const ticks_t ticks) {
    *((ticks_t *) BUS_REG_TIMER) = ticks;
}

void state_update(cpustate_t *const self, const struct StateConfig config) {
    debug_assert(NULL != self);

    switch (config.mode) {
        case CPU_MODE_KERNEL:
            enterKernelMode(self);
            break;

        case CPU_MODE_USER:
            enterUserMode(self);
            break;

        default:
            unreachable();
    }

    setVirtualMemory(self, config.virtualMemoryEnabled);
    setFastInterrupts(self, config.fastInterruptsEnabled);
    setInterrupts(self, config.interruptsEnabled);
}

void state_clear(cpustate_t *const self) {
    debug_assert(NULL != self);
    state_update(self, (struct StateConfig) { .mode=CPU_MODE_KERNEL });
}

/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!    ATTENTION    !!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * The code below contains arch-dependant code!
 *
 * From here on there is the code of the functions that are machine-dependent
 * but can't be implemented in the same way between uARM and uMPS architectures.
 */

memaddr *state_programCounter(cpustate_t *const self) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    return &self->pc;
#elif defined(TARGET_UMPS)
    return &self->pc_epc;
#else
#error "Unknown target architecture"
#endif
}

memaddr state_getStackPointer(const cpustate_t *const self) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    return self->sp;
#elif defined(TARGET_UMPS)
    return self->reg_sp;
#else
#error "Unknown target architecture"
#endif
}

void state_setStackPointer(cpustate_t *const self, const memaddr sp) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    self->sp = sp;
#elif defined(TARGET_UMPS)
    self->reg_sp = sp;
#else
#error "Unknown target architecture"
#endif
}

sysno_t state_getSysNo(const cpustate_t *const self) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    return self->a1;
#elif defined(TARGET_UMPS)
    return self->reg_a0;
#else
#error "Unknown target architecture"
#endif
}

static void enterKernelMode(cpustate_t *const self) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    self->cpsr = ((self->cpsr & STATUS_CLEAR_MODE) | STATUS_SYS_MODE);
#elif defined(TARGET_UMPS)
    self->status &= ~(STATUS_KUp);
#else
#error "Unknown target architecture."
#endif
}

static void enterUserMode(cpustate_t *const self) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    self->cpsr = ((self->cpsr & STATUS_CLEAR_MODE) | STATUS_USER_MODE);
#elif defined(TARGET_UMPS)
    self->status |= STATUS_KUp;
#else
#error "Unknown target architecture."
#endif
}

static void setVirtualMemory(cpustate_t *const self, const bool v) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)
    self->CP15_Control = (v) ? CP15_ENABLE_VM(self->CP15_Control)
                             : CP15_DISABLE_VM(self->CP15_Control);
#elif defined(TARGET_UMPS)
    (v) ? (self->status |= STATUS_VMp) : (self->status &= ~(STATUS_VMp));
#else
#error "Unknown target architecture."
#endif
}

static void setFastInterrupts(cpustate_t *const self, const bool v) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)

    self->cpsr = (v) ? STATUS_ENABLE_TIMER(self->cpsr)
                     : STATUS_DISABLE_TIMER(self->cpsr);

#elif defined(TARGET_UMPS)

#define FIQ_MASK    (STATUS_IM(INTERRUPT_LINE_IPI) | STATUS_IM(INTERRUPT_LINE_CPU_TIMER) | STATUS_IM(INTERRUPT_LINE_INTERVAL_TIMER))
    (v) ? (self->status |= (FIQ_MASK | STATUS_IEp))
        : (self->status &= ~(FIQ_MASK));
#undef FIQ_MASK

#else
#error "Unknown target architecture."
#endif
}

static void setInterrupts(cpustate_t *const self, const bool v) {
    debug_assert(NULL != self);
#if defined(TARGET_UARM)

    self->cpsr = (v) ? STATUS_ENABLE_INT(self->cpsr)
                     : STATUS_DISABLE_INT(self->cpsr);

#elif defined(TARGET_UMPS)

#define IRQ_MASK    (STATUS_IM(INTERRUPT_LINE_DISK) | STATUS_IM(INTERRUPT_LINE_TAPE) | STATUS_IM(INTERRUPT_LINE_ETHERNET) | STATUS_IM(INTERRUPT_LINE_PRINTER) | STATUS_IM(INTERRUPT_LINE_TERMINAL))
    (v) ? (self->status |= (IRQ_MASK | STATUS_IEp))
        : (self->status &= ~(IRQ_MASK));
#undef IRQ_MASK

#else
#error "Unknown target architecture."
#endif
}
