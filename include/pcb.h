#pragma once

#include <primitive_types.h>
#include <listx.h>
#include <core.h>

// Process Control Block (PCB) data structure
typedef struct pcb_t {
    // processor state
    cpustate_t p_s;

    // process queue fields
    struct list_head p_next;

    // process tree fields
    struct list_head p_child, p_sib;
    struct pcb_t *p_parent;

    // custom handlers
    cpustate_t *sysbkHandler;
    cpustate_t *sysbkOldArea;
    cpustate_t *TLBHandler;
    cpustate_t *TLBOldArea;
    cpustate_t *trapHandler;
    cpustate_t *trapOldArea;

    // key of the semaphore on which the process is eventually blocked
    int *p_semkey;

    // priority defined when creating a process
    int original_priority;

    // the current priority of the process: original_priority + aging
    int priority;

    // process execution times
    ticks_t start_time;             // TODLow when process runs for the first time; used to calculate the wallclock time
    ticks_t user_time;              // self-explained
    ticks_t kernel_time;            // self-explained
    ticks_t latest_handler_time;    // sample of timer at the moment of the termination of the last call to a handler
} pcb_t;

// free list handling functions
void initPcbs(void);
void freePcb(struct pcb_t *p);
struct pcb_t *allocPcb(void);

// queue handling functions
void mkEmptyProcQ(struct list_head *head);
int emptyProcQ(struct list_head *head);
void insertProcQ(struct list_head *head, struct pcb_t *p);
struct pcb_t *headProcQ(struct list_head *head);
struct pcb_t *removeProcQ(struct list_head *head);
struct pcb_t *outProcQ(struct list_head *head, struct pcb_t *p);

// tree view functions
int emptyChild(struct pcb_t *root);
void insertChild(struct pcb_t *root, struct pcb_t *p);
struct pcb_t *removeChild(struct pcb_t *root);
struct pcb_t *outChild(struct pcb_t *child);

/**
 * Returns the process control block identifier.
 * The PID is unique and is > 0.
 *
 * @attention (NULL == p) is a checked runtime error.
 * @attention p must be a valid pcb.
 *
 * @return the PID of the specified pcb.
 */
usize getPid(const struct pcb_t *p);
