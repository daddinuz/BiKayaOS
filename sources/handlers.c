#include <core.h>
#include <scheduler.h>
#include <assertions.h>
#include <const_bikaya.h>
#include <handlers.h>

#define ACK_COMMAND         1U
#define READY_STATE         1U
#define BUSY_STATE          3U

static const devreg_t *const FIRST_DEVICE = (const devreg_t *) DEV_REG_ADDR(INTERRUPT_LINE_DISK, 0);
static const devreg_t *const FIRST_TERM = (const devreg_t *) DEV_REG_ADDR(INTERRUPT_LINE_TERMINAL, 0);
static const devreg_t *const LAST_TERM = (const devreg_t *) DEV_REG_ADDR(INTERRUPT_LINE_TERMINAL, MACHINE_DEVICE_TERMINAL_NO);

void handlers_interruptHandler(void) {
    // The interrupt line must be obtained before reseting the timer
    const unsigned il = machine_getInterruptLine();

    const ticks_t timeLeft = machine_resetIntervalTimer();
    cpustate_t *oldState = MACHINE_OLD_INTERRUPT_AREA;

#if defined(TARGET_UARM)
    // restore PC to the correct instruction to be executed
    *state_programCounter(oldState) -= MACHINE_WORD_SIZE;
#endif

    switch (il) {
        case INTERRUPT_LINE_INTERVAL_TIMER:
            scheduler_contextSwitch(oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer());
            unreachable();

        case INTERRUPT_LINE_DISK:
        case INTERRUPT_LINE_TAPE:
        case INTERRUPT_LINE_ETHERNET:
        case INTERRUPT_LINE_PRINTER: {
            dtpreg_t *const device = (dtpreg_t *) DEV_REG_ADDR(il, machine_getInterruptDevice(il));
            device->command = ACK_COMMAND;
            break;
        }

        case INTERRUPT_LINE_TERMINAL: {
            termreg_t *const device = (termreg_t *) DEV_REG_ADDR(il, machine_getInterruptDevice(il));

            if (READY_STATE != (device->transm_status & 0xFFU) && BUSY_STATE != (device->transm_status & 0xFFU)) {
                device->transm_command = ACK_COMMAND;
            }

            if (READY_STATE != (device->recv_status & 0xFFU) && BUSY_STATE != (device->recv_status & 0xFFU)) {
                device->recv_command = ACK_COMMAND;
            }

            break;
        }

        default:
            unreachable();
    }

    scheduler_resume(oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer(), NULL);
}

void handlers_sysbkHandler(void) {
    const ticks_t timeLeft = machine_resetIntervalTimer();
    cpustate_t *oldState = MACHINE_OLD_SYSBK_AREA;
    struct TimeInfo timeInfo = { .userTime = NULL, .kernelTime = NULL, .wallclockTime = NULL };

#if defined(TARGET_UMPS)
    // restore PC to the correct instruction to be executed
    *state_programCounter(oldState) += MACHINE_WORD_SIZE;
#endif

    switch (state_getSysNo(oldState)) {
        case GETCPUTIME: {
            timeInfo.userTime = (ticks_t *) state_getSysArg1(oldState);
            timeInfo.kernelTime = (ticks_t *) state_getSysArg2(oldState);
            timeInfo.wallclockTime = (ticks_t *) state_getSysArg3(oldState);
            break;
        }

        case TERMINATEPROCESS: {
            void *const pid = (void *) state_getSysArg1(oldState);
            scheduler_drop(pid, oldState);
            break;
        }

        case GETPID: {
            const void **const pid = (const void **) state_getSysArg1(oldState);
            const void **const parentPid = (const void **) state_getSysArg2(oldState);

            if (NULL != pid) {
               *pid = scheduler_getCurrentProcess();
            }

            if (NULL != parentPid) {
               *parentPid = scheduler_getCurrentProcessParent();
            }

            break;
        }

        case CREATEPROCESS: {
            const cpustate_t *const childState = (const cpustate_t *) state_getSysArg1(oldState);
            const int priority = (int) state_getSysArg2(oldState);
            const void **const childPid = (const void **) state_getSysArg3(oldState);
            debug_assert(NULL != childState);

            state_setSysReturn(oldState, scheduler_scheduleChild(childState, priority, childPid));
            break;
        }

        case VERHOGEN: {
            int *const key = (int *) state_getSysArg1(oldState);
            debug_assert(NULL != key);

            scheduler_verhogen(key);
            break;
        }

        case PASSEREN: {
            int *const key = (int *) state_getSysArg1(oldState);
            debug_assert(NULL != key);

            scheduler_passeren(key, oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer());
            break;
        }

        case SPECPASSUP: {
            const int sysReturnValue = scheduler_registerCustomHandler((enum ExcType) state_getSysArg1(oldState),
                                                                       (cpustate_t *) state_getSysArg2(oldState),
                                                                       (cpustate_t *) state_getSysArg3(oldState));
            state_setSysReturn(oldState, sysReturnValue);
            if (0 != sysReturnValue) {
                scheduler_drop(NULL, oldState);
                unreachable();
            }

            break;
        }

        case WAITIO: {
            const unsigned command = state_getSysArg1(oldState);
            devreg_t *const device = (devreg_t *) state_getSysArg2(oldState);
            const int subdevice = state_getSysArg3(oldState);

            unsigned *statusRef = NULL;
            unsigned *commandRef = NULL;

            if (FIRST_DEVICE <= device && device < FIRST_TERM) {
                statusRef = &device->dtp.status;
                commandRef = &device->dtp.command;
            } else if (FIRST_TERM <= device && device < LAST_TERM) {
                if (0 == subdevice) {
                    statusRef = &device->term.transm_status;
                    commandRef = &device->term.transm_command;
                } else {
                    statusRef = &device->term.recv_status;
                    commandRef = &device->term.recv_command;
                }
            } else {
                unreachable();
            }

            *commandRef = command;
            while (BUSY_STATE == (*statusRef & 0xFFU));
            state_setSysReturn(oldState, *statusRef);

            break;
        }

        default:
            scheduler_callSysbkHandler(oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer());
            unreachable();
    }

    scheduler_resume(oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer(), &timeInfo);
}

void handlers_TLBHandler(void) {
    const ticks_t timeLeft = machine_resetIntervalTimer();
    cpustate_t *oldState = MACHINE_OLD_TLB_MGMT_AREA;
    scheduler_callTLBHandler(oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer());
    unreachable();
}

void handlers_trapHandler(void) {
    const ticks_t timeLeft = machine_resetIntervalTimer();
    cpustate_t *oldState = MACHINE_OLD_PRGM_TRAP_AREA;
    scheduler_callTrapHandler(oldState, timeLeft, INTERVAL_TIMER_MAX - machine_getIntervalTimer());
    unreachable();
}
