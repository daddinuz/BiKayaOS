#ifndef ASL_H
#define ASL_H

#include <types_bikaya.h>

/* ASL handling functions */
void initASL(void);
struct semd_t *getSemd(int *key);

int insertBlocked(int *key, struct pcb_t *p);
struct pcb_t *headBlocked(int *key);
struct pcb_t *removeBlocked(int *key);
struct pcb_t *outBlocked(struct pcb_t *p);
void outChildBlocked(struct pcb_t *p);

#endif
