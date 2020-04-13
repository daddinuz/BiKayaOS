#include <core.h>
#include <scheduler.h>
#include <handlers.h>

void handlers_interruptHandler(void) {
    cpustate_t *oldState = MACHINE_OLD_INTERRUPT_AREA;
    const unsigned il = machine_getInterruptLine();

#if defined(TARGET_UARM)
    // restore PC to the correct instruction to be executed
    *state_programCounter(oldState) -= MACHINE_WORD_SIZE;
#endif

    switch (il) {
        case INTERRUPT_LINE_INTERVAL_TIMER:
            scheduler_dispatch(oldState);
            unreachable();

        default: // Network, Disks, tapes... NoOp
            unreachable();
    }

    core_loadState(oldState);
}

void handlers_sysbkHandler(void) {
    cpustate_t *oldState = MACHINE_OLD_SYSBK_AREA;

#if defined(TARGET_UMPS)
    // restore PC to the correct instruction to be executed
    *state_programCounter(oldState) += MACHINE_WORD_SIZE;
#endif

    switch (state_getSysNo(oldState)) {
        case SYSNO_TERMINATE_PROCESS: {
            scheduler_freeCurrentProcess();
            scheduler_dispatch(NULL);
            unreachable();
        }

        default:
            unreachable();
    }

    core_loadState(oldState);
}

void handlers_unexpectedHandler(void) {
    unreachable();
}
