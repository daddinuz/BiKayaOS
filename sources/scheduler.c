#include <pcb.h>
#include <asl.h>
#include <core.h>
#include <memory.h>
#include <assertions.h>
#include <scheduler.h>

struct list_head readyQueue = LIST_HEAD_INIT(readyQueue);
struct pcb_t *curProc = NULL;
static int globalAge = 0;

static void dropProcess(struct pcb_t *proc);
static void dropProgeny(struct pcb_t *node);

static inline void updateCurProcTime(const ticks_t timeLeft, const ticks_t handlerTime) {
    debug_assert(NULL != curProc);
    curProc->user_time += curProc->latest_handler_time - timeLeft;
    curProc->kernel_time += handlerTime;
    curProc->latest_handler_time = (handlerTime > timeLeft) ? 0 : timeLeft - handlerTime;
}

int scheduler_scheduleChild(const cpustate_t *const childState, const int priority, const void **const childPid) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != childState);
    struct pcb_t *const childProc = allocPcb();

    if (NULL == childProc) {
       return -1;
    }

    memdup(&childProc->p_s, childState, sizeof(childProc->p_s));
    childProc->priority = childProc->original_priority = priority;
    insertChild(curProc, childProc);
    insertProcQ(&readyQueue, childProc);

    if (NULL != childPid) {
       *childPid = childProc;
    }

    return 0;
}

bool scheduler_scheduleWith(void (*const process)(void), const int priority, const bool interruptsEnabled) {
    debug_assert(NULL != process);
    struct pcb_t *proc = allocPcb();

    if (NULL != proc) {
        cpustate_t *state = &proc->p_s;

        state_update(state, (struct StateConfig) {
            .mode=CPU_MODE_KERNEL,
            .fastInterruptsEnabled=true,
            .interruptsEnabled=interruptsEnabled,
        });
        *state_programCounter(state) = (memaddr) process;
        state_setStackPointer(state, MACHINE_RAM_LIMIT - (MACHINE_STACK_SIZE * getPid(proc)));
        proc->priority = proc->original_priority = priority;
        insertProcQ(&readyQueue, proc);

        return true;
    }

    return false;
}

void scheduler_resume(cpustate_t *const procState, const ticks_t timeLeft, const ticks_t handlerTime, struct TimeInfo *const timeInfo) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != procState);

    updateCurProcTime(timeLeft, handlerTime);
    if (NULL != timeInfo) {
        if (NULL != timeInfo->userTime) *timeInfo->userTime = curProc->user_time;
        if (NULL != timeInfo->kernelTime) *timeInfo->kernelTime = curProc->kernel_time;
        if (NULL != timeInfo->wallclockTime) *timeInfo->wallclockTime = machine_getTODLow() - curProc->start_time;
    }

    machine_setIntervalTimer(curProc->latest_handler_time * machine_getClockResolution());
    core_loadState(procState);
}

void scheduler_dispatch(void) {
    debug_assert(NULL == curProc);
    curProc = removeProcQ(&readyQueue);

    if (NULL == curProc) {
        core_halt();
        unreachable();
    }

    struct pcb_t *iter = NULL;
    list_for_each_entry(iter, &readyQueue, p_next) {
        iter->priority += 1;
    }
    globalAge += 1;

    if (0 == curProc->start_time) {
        curProc->start_time = machine_getTODLow();
    }
    curProc->latest_handler_time = TIME_SLICE;

    machine_setIntervalTimer(TIME_SLICE * machine_getClockResolution());
    core_loadState(&curProc->p_s);
}

void scheduler_contextSwitch(cpustate_t *const procState, const ticks_t timeLeft, const ticks_t handlerTime) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != procState);

    updateCurProcTime(timeLeft, handlerTime);
    curProc->priority = curProc->original_priority;
    memdup(&curProc->p_s, procState, sizeof(*procState));

    insertProcQ(&readyQueue, curProc);
    curProc = NULL;
    scheduler_dispatch();
    unreachable();
}

static void dropProcess(struct pcb_t *const proc) {
    debug_assert(NULL != proc);

    // proc must be in the ready queue or in the blocked queue of a semaphore
    if (NULL == outProcQ(&readyQueue, proc) && NULL == outBlocked(proc)) {
        unreachable();
    }

    freePcb(proc);
}

static void dropProgeny(struct pcb_t *const node) {
    debug_assert(NULL != node);
    struct pcb_t *child = NULL;
    struct pcb_t *tmpChild = NULL;

    list_for_each_entry_reverse(child, &node->p_child, p_child) {
        struct pcb_t *sib = NULL;
        struct pcb_t *tmpSib = NULL;

        if (NULL != tmpChild) {
            dropProcess(tmpChild);
            tmpChild = NULL;
        }

        list_for_each_entry_reverse(sib, &child->p_sib, p_sib) {
            if (NULL != tmpSib) {
                dropProcess(tmpSib);
                tmpSib = NULL;
            }

            dropProgeny(sib);
            tmpSib = sib;
        }

        tmpChild = child;
        if (NULL != tmpSib) {
            dropProcess(tmpSib);
            tmpSib = NULL;
        }
    }

    if (NULL != tmpChild) {
        dropProcess(tmpChild);
        tmpChild = NULL;
    }
}

void scheduler_drop(void *const pid, cpustate_t *const procState) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != procState);
    
    struct pcb_t *const proc = (NULL == pid) ? curProc : pid;
    debug_assert(NULL != proc);
    
    // Suspend current process
    memdup(&curProc->p_s, procState, sizeof(curProc->p_s));
    curProc->priority = MACHINE_MAX_INT;
    insertProcQ(&readyQueue, curProc);

    outChild(proc);
    dropProgeny(proc);
    dropProcess(proc);

    if (headProcQ(&readyQueue) != curProc) {
        curProc = NULL;
        scheduler_dispatch();
        unreachable();
    }

    removeProcQ(&readyQueue);
}

const void *scheduler_getCurrentProcess(void) {
    return curProc;
}

const void *scheduler_getCurrentProcessParent(void) {
    return (NULL == curProc) ? NULL : curProc->p_parent;
}

int scheduler_registerCustomHandler(const enum ExcType type, cpustate_t *const oldArea, cpustate_t *const handler) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != oldArea);
    debug_assert(NULL != handler);

    cpustate_t **oldAreaRef;
    cpustate_t **handlerRef;

    switch (type) {
        case ExcType_Sysbk:
            oldAreaRef = &curProc->sysbkOldArea;
            handlerRef = &curProc->sysbkHandler;
            break;

        case ExcType_TLB:
            oldAreaRef = &curProc->TLBOldArea;
            handlerRef = &curProc->TLBHandler;
            break;

        case ExcType_Trap:
            oldAreaRef = &curProc->trapOldArea;
            handlerRef = &curProc->trapHandler;
            break;

        default: 
            unreachable();
    }

    if (NULL != *oldAreaRef) {
        debug_assert(NULL != *handlerRef);
        return -1;
    }

    debug_assert(NULL == *handlerRef);
    *oldAreaRef = oldArea;
    *handlerRef = handler;
    return 0;
} 

void scheduler_callSysbkHandler(cpustate_t *const procState, const ticks_t timeLeft, const ticks_t handlerTime) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != procState);

    if (NULL != curProc->sysbkHandler) {
        debug_assert(NULL != curProc->sysbkOldArea);

        updateCurProcTime(timeLeft, handlerTime);
        memdup(curProc->sysbkOldArea, procState, sizeof(*procState));
        machine_setIntervalTimer(curProc->latest_handler_time * machine_getClockResolution());
        core_loadState(curProc->sysbkHandler);
    }

    scheduler_drop(NULL, procState);
    unreachable();
}

void scheduler_callTLBHandler(cpustate_t *const procState, const ticks_t timeLeft, const ticks_t handlerTime) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != procState);

    if (NULL != curProc->TLBHandler) {
        debug_assert(NULL != curProc->TLBOldArea);

        updateCurProcTime(timeLeft, handlerTime);
        memdup(curProc->TLBOldArea, procState, sizeof(*procState));
        machine_setIntervalTimer(curProc->latest_handler_time * machine_getClockResolution());
        core_loadState(curProc->TLBHandler);
    }

    scheduler_drop(NULL, procState);
    unreachable();
}

void scheduler_callTrapHandler(cpustate_t *const procState, const ticks_t timeLeft, const ticks_t handlerTime) {
    debug_assert(NULL != curProc);
    debug_assert(NULL != procState);

    if (NULL != curProc->trapHandler) {
        debug_assert(NULL != curProc->trapOldArea);

        updateCurProcTime(timeLeft, handlerTime);
        memdup(curProc->trapOldArea, procState, sizeof(*procState));
        machine_setIntervalTimer(curProc->latest_handler_time * machine_getClockResolution());
        core_loadState(curProc->trapHandler);
    }

    scheduler_drop(NULL, procState);
    unreachable();
}

void scheduler_passeren(int *const semaphoreKey, cpustate_t *const procState, const ticks_t timeLeft, const ticks_t handlerTime) {
    debug_assert(NULL != semaphoreKey);
    debug_assert(NULL != curProc);

    if (0 < *semaphoreKey) {
        *semaphoreKey -= 1;
    } else {
        updateCurProcTime(timeLeft, handlerTime);
        curProc->latest_handler_time = TIME_SLICE;
        curProc->priority = globalAge - curProc->priority;
        memdup(&curProc->p_s, procState, sizeof(curProc->p_s));

        if (0 == insertBlocked(semaphoreKey, curProc)) {
            curProc = NULL;
            scheduler_dispatch();
        }
        
        unreachable();
    }
}

void scheduler_verhogen(int *const semaphoreKey) {
    debug_assert(NULL != semaphoreKey);
    debug_assert(NULL != curProc);

    struct pcb_t *const firstProc = removeBlocked(semaphoreKey);
    if (NULL == firstProc) {
        *semaphoreKey += 1;
    } else {
        firstProc->priority = globalAge - firstProc->priority;
        insertProcQ(&readyQueue, firstProc);
    }
}
