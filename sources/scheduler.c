#include <assertions.h>
#include <pcb.h>
#include <core.h>
#include <memory.h>
#include <scheduler.h>

struct pcb_t *curProc = NULL;
struct list_head readyQueue = LIST_HEAD_INIT(readyQueue);

bool scheduler_schedule(void (*const process)(void), const int priority) {
    debug_assert(NULL != process);
    struct pcb_t *proc = allocPcb();

    if (NULL != proc) {
        cpustate_t *state = &proc->p_s;

        state_update(state, (struct StateConfig) {
            .mode=CPU_MODE_KERNEL,
            .fastInterruptsEnabled=true,
        });
        *state_programCounter(state) = (memaddr) process;
        state_setStackPointer(state, MACHINE_RAM_LIMIT - MACHINE_STACK_SIZE * getpid(proc));
        proc->priority = proc->original_priority = priority;
        insertProcQ(&readyQueue, proc);

        return true;
    }

    return false;
}

void scheduler_dispatch(cpustate_t *const procState) {
    if (NULL != curProc) {
        if (NULL != procState) {
            memdup(&curProc->p_s, procState, sizeof(*procState));
        }
        curProc->priority = curProc->original_priority;
        insertProcQ(&readyQueue, curProc);
    }

    curProc = removeProcQ(&readyQueue);

    if (NULL == curProc) {
        core_halt();
        unreachable();
    }

    struct pcb_t *iter = NULL;
    list_for_each_entry(iter, &readyQueue, p_next) {
        iter->priority += 1;
    }

    /*
        us: microseconds
        hz: time scale (ticks per microseconds)

        us = ticks / hz -> ticks = us * hz -> hz = ticks / us
     */
    machine_setIntervalTimer(TIME_SLICE * machine_getClockResolution());
    core_loadState(&curProc->p_s);
}

void scheduler_freeCurrentProcess(void){
    debug_assert(NULL != curProc);
    freePcb(curProc);
    curProc = NULL;
}
