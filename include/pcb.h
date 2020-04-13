#pragma once

#include <primitive_types.h>
#include <listx.h>
#include <core.h>

// Process Control Block (PCB) data structure
typedef struct pcb_t {
    // process queue fields
    struct list_head p_next;

    // process tree fields
    struct pcb_t *p_parent;
    struct list_head p_child, p_sib;

    // processor state
    cpustate_t p_s;

    // process priority
    int original_priority;
    int priority;

    // key of the semaphore on which the process is eventually blocked
    int *p_semkey;
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
 *
 * @return the PID of the specified pcb.
 */
usize getpid(const struct pcb_t *p);
