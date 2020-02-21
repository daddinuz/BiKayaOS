#ifndef PCB_H
#define PCB_H

#include <types_bikaya.h>

/* PCB handling functions */

/* PCB free list handling functions */
void initPcbs(void);
void freePcb(struct pcb_t *p);
struct pcb_t *allocPcb(void);

/* PCB queue handling functions */
void mkEmptyProcQ(struct list_head *head);
int emptyProcQ(struct list_head *head);
void insertProcQ(struct list_head *head, struct pcb_t *p);
struct pcb_t *headProcQ(struct list_head *head);
struct pcb_t *removeProcQ(struct list_head *head);
struct pcb_t *outProcQ(struct list_head *head, struct pcb_t *p);

/* Tree view functions */
int emptyChild(struct pcb_t *this);
void insertChild(struct pcb_t *prnt, struct pcb_t *p);
struct pcb_t *removeChild(struct pcb_t *p);
struct pcb_t *outChild(struct pcb_t *p);

#endif
