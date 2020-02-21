/*****************************************************************************
 * Copyright 2004, 2005 Michael Goldweber, Davide Brini.                     *
 * Modified 2012 by Marco Di Felice                                          *
 * Modified 2018 by Renzo Davoli
 *                                                                           *
 * This file is part of BIKAYA.                                              *
 *                                                                           *
 * bikaya is free software;you can redistribute it and/or modify it under the*
 * terms of the GNU General Public License as published by the Free Software *
 * Foundation; either version 2 of the License, or (at your option) any      *
 * later version.                                                            *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.                  *
 *****************************************************************************/

/*********************************P1TEST.C*******************************
 *
 *	Test program for the modules ASL and PCB (phase 1).
 *
 *	Produces progress messages on terminal 0 in addition
 *		to the array ``okbuf[]''
 *		Error messages will also appear on terminal 0 in
 *		addition to the array ``errbuf[]''.
 *
 *		Aborts as soon as an error is detected.
 *
 *
 */

#include <const.h>
#include <listx.h>

#ifdef TARGET_UMPS
#include <umps/libumps.h>
#include <umps/arch.h>
#endif
#ifdef TARGET_UARM
#include <uarm/libuarm.h>
#include <uarm/arch.h>
#endif

#include <pcb.h>
#include <asl.h>

#define MAXSEM MAXPROC

#define MAX_PCB_PRIORITY     10
#define MIN_PCB_PRIORITY     0
#define DEFAULT_PCB_PRIORITY 5

char okbuf[2048]; /* sequence of progress messages */
char errbuf[128]; /* contains reason for failing */
char msgbuf[128]; /* nonrecoverable error message before shut down */



int     onesem;
pcb_t * procp[MAXPROC], *p, *q, *maxproc, *minproc, *proc;
semd_t *semd[MAXSEM];
int     sem[MAXSEM + 1];

struct list_head qa;
char *           mp = okbuf;

#define ST_READY       1
#define ST_BUSY        3
#define ST_TRANSMITTED 5

#define CMD_ACK      1
#define CMD_TRANSMIT 2

#define CHAR_OFFSET      8
#define TERM_STATUS_MASK 0xFF


/******************************************************************************
 * I/O Routines to write on a terminal
 ******************************************************************************/

/* This function returns the terminal transmitter status value given its address */
static unsigned int tx_status(termreg_t *tp) {
    return ((tp->transm_status) & TERM_STATUS_MASK);
}

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint(char *str, unsigned int term) {
    termreg_t *term_reg;

    unsigned int stat;
    unsigned int cmd;

    unsigned int error = FALSE;

    if (term < DEV_PER_INT) {
        term_reg = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, term);

        /* test device status */
        stat = tx_status(term_reg);
        if ((stat == ST_READY) || (stat == ST_TRANSMITTED)) {
            /* device is available */

            /* print cycle */
            while ((*str != '\0') && (!error)) {
                cmd                      = (*str << CHAR_OFFSET) | CMD_TRANSMIT;
                term_reg->transm_command = cmd;

                /* busy waiting */
                while ((stat = tx_status(term_reg)) == ST_BUSY)
                    ;

                /* end of wait */
                if (stat != ST_TRANSMITTED) {
                    error = TRUE;
                } else {
                    /* move to next char */
                    str++;
                }
            }
        } else {
            /* device is not available */
            error = TRUE;
        }
    } else {
        /* wrong terminal device number */
        error = TRUE;
    }

    return (!error);
}


/* This function places the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void addokbuf(char *strp) {

    termprint(strp, 0);
}


/* This function places the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void adderrbuf(char *strp) {

    termprint(strp, 0);

    PANIC();
}


/******************************************************************************
 * Main Test File
 ******************************************************************************/


int main() {
    int i;

    initPcbs();
    addokbuf("Initialized Process Control Blocks   \n");

    /* Check allocPcb */
    for (i = 0; i < MAXPROC; i++) {
        if ((procp[i] = allocPcb()) == NULL)
            adderrbuf("allocPcb(): unexpected NULL   ");
    }

    if (allocPcb() != NULL) {
        adderrbuf(" ERROR: allocPcb(): allocated more than MAXPROC entries   ");
    }
    addokbuf(" allocPcb test OK   \n");


    /* Return the last 10 entries back to free list */
    for (i = 10; i < MAXPROC; i++)
        freePcb(procp[i]);

    addokbuf(" Added 10 entries to the free PCB list   \n");

    /* Create a 10-element process queue */
    INIT_LIST_HEAD(&qa);

    if (!emptyProcQ(&qa))
        adderrbuf("ERROR: emptyProcQ(qa): unexpected FALSE   ");

    addokbuf("Testing insertProcQ ...   \n");

    for (i = 0; i < 10; i++) {
        if ((q = allocPcb()) == NULL)
            adderrbuf("ERROR: allocPcb(): unexpected NULL while insert   ");
        switch (i) {
            case 3:
                q->priority = DEFAULT_PCB_PRIORITY;
                proc        = q;
                break;
            case 4:
                q->priority = MAX_PCB_PRIORITY;
                maxproc     = q;
                break;
            case 5:
                q->priority = MIN_PCB_PRIORITY;
                minproc     = q;
                break;
            default:
                q->priority = DEFAULT_PCB_PRIORITY;
                break;
        }
        insertProcQ(&qa, q);
    }

    addokbuf("Test insertProcQ: OK. Inserted 10 elements \n");

    if (emptyProcQ(&qa))
        adderrbuf("ERROR: emptyProcQ(qa): unexpected TRUE");

    /* Check outProcQ and headProcQ */
    if (headProcQ(&qa) != maxproc)
        adderrbuf("ERROR: headProcQ(qa) failed   ");

    /* Removing an element from ProcQ */
    q = outProcQ(&qa, proc);
    if ((q == NULL) || (q != proc))
        adderrbuf("ERROR: outProcQ(&qa, proc) failed to remove the entry   ");
    freePcb(q);

    /* Removing the first element from ProcQ */
    q = removeProcQ(&qa);
    if (q == NULL || q != maxproc)
        adderrbuf("ERROR: removeProcQ(&qa, midproc) failed to remove the elements in the right order   ");
    freePcb(q);

    /* Removing other 7 elements  */
    addokbuf(" Testing removeProcQ ...   \n");
    for (i = 0; i < 7; i++) {
        if ((q = removeProcQ(&qa)) == NULL)
            adderrbuf("removeProcQ(&qa): unexpected NULL   ");
        freePcb(q);
    }

    // Removing the last element
    q = removeProcQ(&qa);
    if (q != minproc)
        adderrbuf("ERROR: removeProcQ(): failed on last entry   ");
    freePcb(q);

    if (removeProcQ(&qa) != NULL)
        adderrbuf("ERROR: removeProcQ(&qa): removes too many entries   ");

    if (!emptyProcQ(&qa))
        adderrbuf("ERROR: emptyProcQ(qa): unexpected FALSE   ");

    addokbuf(" Test insertProcQ(), removeProcQ() and emptyProcQ(): OK   \n");
    addokbuf(" Test process queues module: OK      \n");

    addokbuf(" Testing process trees...\n");

    if (!emptyChild(procp[2]))
        adderrbuf("ERROR: emptyChild: unexpected FALSE   ");

    /* make procp[1],procp[2],procp[3], procp[7] children of procp[0] */
    addokbuf("Inserting...   \n");
    insertChild(procp[0], procp[1]);
    insertChild(procp[0], procp[2]);
    insertChild(procp[0], procp[3]);
    insertChild(procp[0], procp[7]);
    addokbuf("Inserted 2 children of pcb0  \n");

    /* make procp[8],procp[9] children of procp[7] */
    insertChild(procp[7], procp[8]);
    insertChild(procp[7], procp[9]);
    addokbuf("Inserted 2 children of pcb7  \n");

    if (emptyChild(procp[0]))
        adderrbuf("ERROR: emptyChild(procp[0]): unexpected TRUE   ");

    if (emptyChild(procp[7]))
        adderrbuf("ERROR: emptyChild(procp[0]): unexpected TRUE   ");

    /* Check outChild */
    q = outChild(procp[1]);
    if (q == NULL || q != procp[1])
        adderrbuf("ERROR: outChild(procp[1]) failed ");

    q = outChild(procp[8]);
    if (q == NULL || q != procp[8])
        adderrbuf("ERROR: outChild(procp[8]) failed ");

    /* Check removeChild */
    q = removeChild(procp[0]);
    if (q == NULL || q != procp[2])
        adderrbuf("ERROR: removeChild(procp[0]) failed ");

    q = removeChild(procp[7]);
    if (q == NULL || q != procp[9])
        adderrbuf("ERROR: removeChild(procp[7]) failed ");

    q = removeChild(procp[0]);
    if (q == NULL || q != procp[3])
        adderrbuf("ERROR: removeChild(procp[0]) failed ");

    q = removeChild(procp[0]);
    if (q == NULL || q != procp[7])
        adderrbuf("ERROR: removeChild(procp[0]) failed ");


    if (removeChild(procp[0]) != NULL)
        adderrbuf("ERROR: removeChild(): removes too many children   ");

    if (!emptyChild(procp[0]))
        adderrbuf("ERROR: emptyChild(procp[0]): unexpected FALSE   ");

    addokbuf("Test: insertChild(), removeChild() and emptyChild() OK   \n");
    addokbuf("Testing process tree module OK      \n");


    freePcb(procp[0]);
    freePcb(procp[1]);
    freePcb(procp[2]);
    freePcb(procp[3]);
    freePcb(procp[4]);
    freePcb(procp[5]);
    freePcb(procp[6]);
    freePcb(procp[7]);
    freePcb(procp[8]);
    freePcb(procp[9]);


    /* check ASL */
    initASL();
    addokbuf("Initializing active semaphore list   \n");

    /* check removeBlocked and insertBlocked */
    addokbuf(" Test insertBlocked(): test #1 started  \n");
    for (i = 10; i < MAXPROC; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[i], procp[i]))
            adderrbuf("ERROR: insertBlocked() test#1: unexpected TRUE   ");
    }

    addokbuf("Test insertBlocked(): test #2 started  \n");
    for (i = 0; i < 10; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[i], procp[i]))
            adderrbuf("ERROR:insertBlocked() test #2: unexpected TRUE   ");
    }

    /* check if semaphore descriptors are returned to the free list */
    p = removeBlocked(&sem[11]);
    if (insertBlocked(&sem[11], p))
        adderrbuf("ERROR: removeBlocked(): fails to return to free list   ");

    if (insertBlocked(&sem[MAXSEM], procp[9]) == FALSE)
        adderrbuf("ERROR: insertBlocked(): inserted more than MAXPROC   ");

    addokbuf("Test removeBlocked(): test started   \n");
    for (i = 10; i < MAXPROC; i++) {
        q = removeBlocked(&sem[i]);
        if (q == NULL)
            adderrbuf("ERROR: removeBlocked(): wouldn't remove   ");
        if (q != procp[i])
            adderrbuf("ERROR: removeBlocked(): removed wrong element   ");
    }

    if (removeBlocked(&sem[11]) != NULL)
        adderrbuf("ERROR: removeBlocked(): removed nonexistent blocked proc   ");

    addokbuf("Test insertBlocked() and removeBlocked() ok   \n");

    if (headBlocked(&sem[11]) != NULL)
        adderrbuf("ERROR: headBlocked(): nonNULL for a nonexistent queue   ");

    if ((q = headBlocked(&sem[9])) == NULL)
        adderrbuf("ERROR: headBlocked(1): NULL for an existent queue   ");
    if (q != procp[9])
        adderrbuf("ERROR: headBlocked(1): wrong process returned   ");
    p = outBlocked(q);
    if (p != q)
        adderrbuf("ERROR: outBlocked(1): couldn't remove from valid queue   ");

    /* Creating a 2-layer tree */
    insertChild(procp[0], procp[1]);
    insertChild(procp[0], procp[2]);
    insertChild(procp[0], procp[3]);
    insertChild(procp[3], procp[4]);

    /* Testing outChildBlocked */
    outChildBlocked(procp[0]);

    if (headBlocked(&sem[0]) != NULL)
        adderrbuf("ERROR: outChildBlocked(): nonNULL for a nonexistent queue (0)  ");
    if (headBlocked(&sem[1]) != NULL)
        adderrbuf("ERROR: outChildBlocked(): nonNULL for a nonexistent queue (1)  ");
    if (headBlocked(&sem[2]) != NULL)
        adderrbuf("ERROR: outChildBlocked(): nonNULL for a nonexistent queue  (2) ");
    if (headBlocked(&sem[3]) != NULL)
        adderrbuf("ERROR: outChildBlocked(): nonNULL for a nonexistent queue (3)  ");
    if (headBlocked(&sem[4]) != NULL)
        adderrbuf("ERROR: outChildBlocked(): nonNULL for a nonexistent queue (4)  ");
    if (headBlocked(&sem[5]) == NULL)
        adderrbuf("ERROR: outChildBlocked(): NULL for an existent queue  (5) ");

    addokbuf("Test headBlocked() and outBlocked(): OK   \n");

    addokbuf("ASL module OK   \n");
    addokbuf("So Long and Thanks for All the Fish\n");

    return 0;
}
