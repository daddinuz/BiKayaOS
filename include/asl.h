#pragma once

#include <listx.h>
#include <pcb.h>

// Semaphore Descriptor (SEMD) data structure
typedef struct semd_t {
    struct list_head s_next;

    // Semaphore key
    int *s_key;

    // Queue of PCBs blocked on the semaphore
    struct list_head s_procQ;
} semd_t;

// ASL handling functions
void initASL(void);
struct semd_t *getSemd(int *key);

int insertBlocked(int *key, struct pcb_t *p);
struct pcb_t *headBlocked(int *key);
struct pcb_t *removeBlocked(int *key);
struct pcb_t *outBlocked(struct pcb_t *p);
void outChildBlocked(struct pcb_t *p);
