#pragma once

#include <primitive_types.h>
#include <core.h>

// time slice in microseconds
#define TIME_SLICE    3000

/**
 * Schedules a process with a given priority.
 *
 * @attention process must exit with SYSCALL(3, 0, 0, 0) otherwise is URE
 *
 * @attention process == NULL is CRE
 *
 */
extern bool scheduler_schedule(void (*process)(void), int priority);

/**
 * Stops the current process saving its state (if any) and starts another process
 * (or halts the machine) following the scheduling policy.
 */
extern void scheduler_dispatch(cpustate_t *procState);

/**
 * Deallocates the current process.
 *
 * @attention There must be a running process or else is CRE
 *
 */
extern void scheduler_freeCurrentProcess(void);
