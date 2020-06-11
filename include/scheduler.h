#pragma once

#include <core.h>
#include <const_bikaya.h>
#include <primitive_types.h>

// time slice in microseconds
#define TIME_SLICE  3000

/**
 * Schedules a process with a given priority.
 *
 * @attention process == NULL is CRE.
 * @attention process must exit with TERMINATEPROCESS syscall otherwise is URE.
 */
extern bool scheduler_scheduleWith(void (*process)(void), int priority, bool interruptsEnabled);

#define scheduler_schedule(...) scheduler_scheduleWith(__VA_ARGS__, false)

/**
 * Schedules a child process for the current process with a given priority.
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention childState == NULL is CRE.
 * @attention current process is NULL is CRE.
 *
 * @return If allocation is successful returns 0, else -1.
 */
extern int scheduler_scheduleChild(const cpustate_t *childState, int priority, const void **childPid);

/**
 * Runs the first process waiting in the ready queue. If the ready queue is empty,
 * it halts the machine.
 *
 * @attention There must be no running process or else is CRE.
 */
extern void scheduler_dispatch(void);

/**
 * Stops the current process saving its state and starts another process
 * (or halts the machine) following the scheduling policy.
 * 
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention There must be a running process or else is CRE.
 * @attention procState == NULL is CRE.
 * @attention passing a state different from the state of the current state is UB.
 *
 * @param procState The most updated state of the current process (obtained inside the handler). 
 * @param timeLeft The remaining part of the time slice for the current process (obtained inside the handler).
 * @param handlerTime The time spent inside the handler
 */
extern void scheduler_contextSwitch(cpustate_t *procState, ticks_t timeLeft, ticks_t handlerTime);

/**
 * Updates timeInfo of the current process and resumes it.
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention There must be a running process or else is CRE.
 * @attention procState == NULL is CRE.
 * @attention passing a state different from the current process' one is UB.
 *
 * @param procState The most updated state of the current process (obtained inside the handler). 
 * @param timeLeft The remaining part of the time slice for the current process (obtained inside the handler).
 * @param handlerTime The time spent inside the handler
 */
extern void scheduler_resume(cpustate_t *procState, ticks_t timeLeft, ticks_t handlerTime, struct TimeInfo *timeInfo);

/**
 * Deallocates the specified process and its progeny.
 * If NULL == pid, it drops the current process.
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention NULL == procState is CRE.
 * @attention NULL == pid and no current process is CRE.
 * @attention If after this call the current process is dropped, then scheduler_dispatch() is called.
 *
 * @param pid The identifier of the process to drop.
 * @param procState The most updated state of the current process (obtained inside the handler). 
 */
extern void scheduler_drop(void *pid, cpustate_t *procState);

/**
 * Returns the process identifier of the current process.
 * 
 * In case there is no current process, NULL is returned.
 */
extern const void *scheduler_getCurrentProcess(void);

/**
 * Returns the process identifier of the parent process of the current.
 *
 * In case there is no current process, NULL is returned.
 * In case the current process has no parent, NULL is returned.
 */
extern const void *scheduler_getCurrentProcessParent(void);

/**
 * Registers a custom handler for the given Exception Type for the current process.
 *
 * @attention oldArea == NULL or handler == NULL is CRE.
 *
 * @param oldArea The area in which is saved the current process state.
 * @param handler The area in which will be saved the custom handler for the current process.
 *
 * @return 0 on success otherwise -1 (in case a custom handler is already registered).
 */
extern int scheduler_registerCustomHandler(enum ExcType type, cpustate_t *oldArea, cpustate_t *handler);

/**
 * Calls the custom handler for System calls for the current process. 
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention procState == NULL is CRE.
 * @attention There must be a running process or else is CRE.
 * @attention Passing a state different from the current process' one is UB.
 * @attention If there is no registered custom handler for the current process results in kernel PANIC.
 *
 * @param procState The most updated state of the current process (obtained inside the handler). 
 * @param timeLeft The remaining part of the time slice for the current process (obtained inside the handler).
 * @param handlerTime The time spent inside the handler
 */
extern void scheduler_callSysbkHandler(cpustate_t *procState, ticks_t timeLeft, ticks_t handlerTime);

/**
 * Calls the custom handler for the TLBs exceptions for the current process. 
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention procState == NULL is CRE.
 * @attention There must be a running process or else is CRE.
 * @attention Passing a state different from the current process' one is UB.
 * @attention If there is no registered custom handler for the current process results in kernel PANIC.
 *
 * @param procState The most updated state of the current process (obtained inside the handler). 
 * @param timeLeft The remaining part of the time slice for the current process (obtained inside the handler).
 * @param handlerTime The time spent inside the handler
 */
extern void scheduler_callTLBHandler(cpustate_t *procState, ticks_t timeLeft, ticks_t handlerTime);

/**
 * Calls the custom handler for the Traps for the current process. 
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention procState == NULL is CRE.
 * @attention There must be a running process or else is CRE.
 * @attention Passing a state different from the current process' one is UB.
 * @attention If there is no registered custom handler for the current process results in kernel PANIC.
 *
 * @param procState The most updated state of the current process (obtained inside the handler). 
 * @param timeLeft The remaining part of the time slice for the current process (obtained inside the handler).
 * @param handlerTime The time spent inside the handler
 */
extern void scheduler_callTrapHandler(cpustate_t *procState, ticks_t timeLeft, ticks_t handlerTime);

/**
 * Performs the passeren on the specified semaphore updating the current process state, its user and kernel time.
 *  
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention NULL == semaphoreKey is CRE.
 * @attention There must be a running process or else is CRE.
 * @attention If the process gets blocked on the semaphore, another process will be dispatched.
 *
 * @param procState The most updated state of the current process (obtained inside the handler). 
 * @param timeLeft The remaining part of the time slice for the current process (obtained inside the handler).
 * @param handlerTime The time spent inside the handler
 */
extern void scheduler_passeren(int *semaphoreKey, cpustate_t *procState, ticks_t timeLeft, ticks_t handlerTime);

/**
 * Performs the verhogen on the specified semaphore. 
 *
 * @attention This function must be called inside a handler, otherwise is UB.
 * @attention NULL == semaphoreKey is CRE.
 * @attention There must be a running process or else is CRE.
 */
extern void scheduler_verhogen(int *semaphoreKey);
