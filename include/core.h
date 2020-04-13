#pragma once

/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!    ATTENTION    !!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * The code below contains arch-dependant code!
 *
 * The main target of BiKaya is portability between architectures.
 * In this header, we designed a layer that abstracts from the underlying architecture.
 * However, this layer exposes some specific symbols of the architectures which are
 * strongly recommended not to use. By using our layer which is a generic (machine-independent)
 * interface, the obtained code can be easily ported on different architectures.
 * Thus, every symbol must be treated as opaque, so they must not be accessed directly,
 * but only through the functions defined in this header.
 *
 * Each supported target must define the following types and consts according
 * to its underlying architecture:
 *
 * - ticks_t                       : clock ticks type.
 * - sysno_t                       : syscall number type.
 * - memaddr                       : memory address type.
 * - cpustate_t                    : self-explaining.
 * - MACHINE_OLD_SYSBK_AREA        : area in which is stored the old CPU state before handling sysbk.
 * - MACHINE_NEW_SYSBK_AREA        : area of the new CPU state when handling sysbk.
 * - MACHINE_OLD_INTERRUPT_AREA    : area in which is stored the old CPU state before handling interrupts.
 * - MACHINE_NEW_INTERRUPT_AREA    : area of the new CPU state when handling interrupts
 * - MACHINE_OLD_PRGM_TRAP_AREA    : area in which is stored the old CPU state before handling program traps.
 * - MACHINE_NEW_PRGM_TRAP_AREA    : area of the new CPU state when handling program traps.
 * - MACHINE_OLD_TLB_MGMT_AREA     : area in which is stored the old CPU state before handling translation lookaside buffers.
 * - MACHINE_NEW_TLB_MGMT_AREA     : area of the new CPU state when handling translation lookaside buffers.
 * - MACHINE_RAM_LIMIT             : self-explaining.
 * - MACHINE_STACK_SIZE            : functions stack size.
 * - MACHINE_WORD_SIZE             : self-explaining.
 * - MACHINE_DEVICE_PRINTER_NO     : number of printer devices that machine can handle.
 * - MACHINE_DEVICE_TERMINAL_NO    : number of terminal devices that machine can handle.
 * - INTERRUPT_LINE_IPI            : interrupt line that indicates inter processor interrupts.
 * - INTERRUPT_LINE_CPU_TIMER      : interrupt line that indicates that the CPU timer caused the interruption.
 * - INTERRUPT_LINE_INTERVAL_TIMER : interrupt line that indicates that the interval timer caused the interruption.
 * - INTERRUPT_LINE_DISK           : interrupt line that indicates that the disk caused the interruption.
 * - INTERRUPT_LINE_TAPE           : interrupt line that indicates that the tape caused the interruption.
 * - INTERRUPT_LINE_ETHERNET       : interrupt line that indicates that the ethernet caused the interruption.
 * - INTERRUPT_LINE_PRINTER        : interrupt line that indicates that the printer caused the interruption.
 * - INTERRUPT_LINE_TERMINAL       : interrupt line that indicates that the terminal caused the interruption.
 * - SYSNO_TERMINATE_PROCESS       : system call terminates process number.
 */

#if defined(TARGET_UARM)

#include <uarm/libuarm.h>
#include <uarm/arch.h>
#include <uarm/uARMtypes.h>

typedef unsigned ticks_t;
typedef unsigned sysno_t;
typedef unsigned memaddr;
typedef state_t cpustate_t;

#define MACHINE_OLD_SYSBK_AREA        ((cpustate_t *) SYSBK_OLDAREA)
#define MACHINE_NEW_SYSBK_AREA        ((cpustate_t *) SYSBK_NEWAREA)

#define MACHINE_OLD_INTERRUPT_AREA    ((cpustate_t *) INT_OLDAREA)
#define MACHINE_NEW_INTERRUPT_AREA    ((cpustate_t *) INT_NEWAREA)

#define MACHINE_OLD_PRGM_TRAP_AREA    ((cpustate_t *) PGMTRAP_OLDAREA)
#define MACHINE_NEW_PRGM_TRAP_AREA    ((cpustate_t *) PGMTRAP_NEWAREA)

#define MACHINE_OLD_TLB_MGMT_AREA     ((cpustate_t *) TLB_OLDAREA)
#define MACHINE_NEW_TLB_MGMT_AREA     ((cpustate_t *) TLB_NEWAREA)

#define MACHINE_RAM_LIMIT             ((unsigned) RAM_TOP)
#define MACHINE_STACK_SIZE            ((unsigned) FRAMESIZE)

#define MACHINE_WORD_SIZE             ((unsigned) WORD_SIZE)

#define MACHINE_DEVICE_PRINTER_NO     ((unsigned) N_DEV_PER_IL)
#define MACHINE_DEVICE_TERMINAL_NO    ((unsigned) N_DEV_PER_IL)

#define INTERRUPT_LINE_IPI            IL_IPI
#define INTERRUPT_LINE_CPU_TIMER      IL_CPUTIMER
#define INTERRUPT_LINE_INTERVAL_TIMER IL_TIMER
#define INTERRUPT_LINE_DISK           IL_DISK
#define INTERRUPT_LINE_TAPE           IL_TAPE
#define INTERRUPT_LINE_ETHERNET       IL_ETHERNET
#define INTERRUPT_LINE_PRINTER        IL_PRINTER
#define INTERRUPT_LINE_TERMINAL       IL_TERMINAL

#define SYSNO_TERMINATE_PROCESS       3

#elif defined(TARGET_UMPS)

#include <umps/libumps.h>
#include <umps/arch.h>
#include <umps/types.h>
#include <umps/cp0.h>

typedef unsigned ticks_t;
typedef unsigned sysno_t;
typedef unsigned memaddr;
typedef state_t cpustate_t;

#define MACHINE_OLD_SYSBK_AREA        ((cpustate_t *) 0x20000348)
#define MACHINE_NEW_SYSBK_AREA        ((cpustate_t *) 0x200003D4)

#define MACHINE_OLD_INTERRUPT_AREA    ((cpustate_t *) 0x20000000)
#define MACHINE_NEW_INTERRUPT_AREA    ((cpustate_t *) 0x2000008C)

#define MACHINE_OLD_PRGM_TRAP_AREA    ((cpustate_t *) 0x20000230)
#define MACHINE_NEW_PRGM_TRAP_AREA    ((cpustate_t *) 0x200002BC)

#define MACHINE_OLD_TLB_MGMT_AREA     ((cpustate_t *) 0x20000118)
#define MACHINE_NEW_TLB_MGMT_AREA     ((cpustate_t *) 0x200001A4)

#define MACHINE_RAM_LIMIT             ((unsigned) ((*((unsigned *) BUS_REG_RAM_BASE)) + (*((unsigned *) BUS_REG_RAM_SIZE))))
#define MACHINE_STACK_SIZE            1024U

#define MACHINE_WORD_SIZE             ((unsigned) WORD_SIZE)

#define MACHINE_DEVICE_PRINTER_NO     ((unsigned) N_DEV_PER_IL)
#define MACHINE_DEVICE_TERMINAL_NO    ((unsigned) N_DEV_PER_IL)

#define INTERRUPT_LINE_IPI            IL_IPI
#define INTERRUPT_LINE_CPU_TIMER      IL_CPUTIMER
#define INTERRUPT_LINE_INTERVAL_TIMER IL_TIMER
#define INTERRUPT_LINE_DISK           IL_DISK
#define INTERRUPT_LINE_TAPE           IL_TAPE
#define INTERRUPT_LINE_ETHERNET       IL_ETHERNET
#define INTERRUPT_LINE_PRINTER        IL_PRINTER
#define INTERRUPT_LINE_TERMINAL       IL_TERMINAL

#define SYSNO_TERMINATE_PROCESS       3

#else
#error "Unknown target architecture"
#endif

#include <primitive_types.h>

#define noreturn _Noreturn

/**
 * Boots the kernel, disables all interrupts and disables virtual memory.
 */
extern void core_boot(void);

/**
 * Stores processor state.
 *
 * @attention (NULL == out) is a checked runtime error.
 * @attention This function leaves the program counter set to 0.
 *
 * @param out Where the processor state will be stored.
 */
extern void core_storeState(cpustate_t *out);

/**
 * Loads the processor state.
 *
 * @attention (NULL == state) is a checked runtime error.
 *
 * @param state New state that has to be loaded.
 */
extern void core_loadState(cpustate_t *state);

/**
 * Halts the execution.
 */
extern noreturn void core_halt(void);

/**
 * Stops the execution. It is used when an error occurs.
 */
extern noreturn void core_panic(void);

/**
 * Alias for core_panic. This function panics and is used as control method
 * since it will never be executed.
 */
#define unreachable() core_panic()

/**
 * Gets the first (by priority) interrupt line active.
 *
 * @attention if no interrupt has been raised, it causes kernel PANIC.
 *
 * @return the interrupt line number.
 */
extern unsigned machine_getInterruptLine(void);

/**
 * Gets the (first) device of the interrupt line that caused the interrupt.
 *
 * @attention if no device has raised an interrupt, it causes kernel PANIC.
 *
 * @param il The interrupt line.
 * @return the device number.
 */
extern unsigned machine_getInterruptDevice(unsigned il);

/**
 * Returns instructions per microseconds.
 */
extern ticks_t machine_getClockResolution(void);

/**
 * Interval timer getter.
 */
extern ticks_t machine_getIntervalTimer(void);

/**
 * Interval timer setter.
 */
extern void machine_setIntervalTimer(ticks_t ticks);

/* CPU state interface. */

/// CPU modes
enum CPUMode {
    CPU_MODE_KERNEL,
    CPU_MODE_USER,
};

struct StateConfig {
    enum CPUMode mode;
    bool virtualMemoryEnabled: 1;
    bool fastInterruptsEnabled: 1;
    bool interruptsEnabled: 1;
};

/**
 * Updates CPU state according to configs.
 *
 * @attention (NULL == self) is a checked runtime error.
 */
extern void state_update(cpustate_t *self, struct StateConfig config);

/**
 * Sets the CPU state to kernel mode, all interrupts disabled
 * and virtual address translation off.
 *
 * @attention (NULL == self) is a checked runtime error.
 */
extern void state_clear(cpustate_t *self);

/**
 *  Gets state's program counter reference.
 *
 *  @attention (NULL == self) is a checked runtime error.
 */
extern memaddr *state_programCounter(cpustate_t *self);

/**
 *  Returns state's stack pointer.
 *
 *  @attention (NULL == self) is a checked runtime error.
 */
extern memaddr state_getStackPointer(const cpustate_t *self);

/**
 *  Sets state's stack pointer.
 *
 *  @attention (NULL == self) is a checked runtime error.
 */
extern void state_setStackPointer(cpustate_t *self, memaddr sp);

/**
 * Returns system call identifier.
 *
 * @attention Calling this function outside the sistem call handler is UB.
 * @attention (NULL == self) is a checked runtime error.
 */
extern sysno_t state_getSysNo(const cpustate_t *self);
