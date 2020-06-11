/*****************************************************************************
 * Copyright 2019, Renzo Davoli, Mattia Maldini                              *
 * Copyright 2004, 2005 Michael Goldweber, Davide Brini.                     *
 *****************************************************************************/

/*********************************P2TEST.C*******************************
 *
 *	Test program for the Bikaya Kernel: phase 2.
 *
 *	Produces progress messages on Terminal0.
 *
 *	This is pretty convoluted code, so good luck!
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 *      Modified by Davide Brini on Nov 26, 2004
 *      Modified by Renzo Davoli 2010
 *      Modified by Miro Mannino on May 8, 2010
 *      Modified by Mattia Maldini, Renzo Davoli 2020
 */

#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#define FRAME_SIZE 4096
/* Elapsed clock ticks (CPU instructions executed) since system power on.
   Only the "low" part is actually used. */
#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018

#define VMON  0x07000000
#define VMOFF (~VMON)

#define getTODLO() (*((unsigned int *)BUS_TODLOW))

#define SP(s)                    s.reg_sp
#define PC(s)                    s.pc_epc
#define VM(s)                    s.status
#define REG0(s)                  s.reg_a0
#define STATUS_ALL_INT_ENABLE(x) (x | (0xFF << 8) | 4U)
#define CAUSE_EXCCODE_GET(cause) ((cause >> 2) & 0x1F)
#define CAUSE_CODE(s)            CAUSE_EXCCODE_GET(s.cause)

#endif

#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMtypes.h"

#define VMON  0x00000001
#define VMOFF (~VMON)

#define CAUSE_CODE(s) CAUSE_EXCCODE_GET(s.CP15_Cause)
#define SP(s)         s.sp
#define PC(s)         s.pc
#define VM(s)         s.CP15_Control
#define REG0(s)       s.a1
#endif

#include "const_bikaya.h"
#include "types_bikaya.h"

typedef unsigned int devregtr;
typedef unsigned int cpu_t;
typedef unsigned int pid_t;

/* if these are not defined */
/* typedef U32 cpu_t; */
/* typedef U32 pid_t; */

/* hardware constants */
#define PRINTCHR 2
#define BYTELEN  8
#define RECVD    5
#define TRANSM   5

#define TERMSTATMASK 0xFF
#define TERMCHARMASK 0xFF00

#define MINLOOPTIME 1000
#define LOOPNUM     10000

#define BADADDR 0xFFFFFFFF /* could be 0x00000000 as well */

/* Software and other constants */
#define ERROR -1

/* just to be clear */
#define NOLEAVES 4 /* number of leaves of p7 process tree */
#define MAXSEM   20


int term_mut = 1,   /* for mutual exclusion on terminal */
    s[MAXSEM + 1],  /* semaphore array */
    testsem    = 0, /* for a simple test */
    startp2    = 0, /* used to start p2 */
    endp2      = 0, /* used to signal p2's demise */
    blkp3      = 1, /* used to block second incaration of p3 */
    synp3      = 0, /* used to allow p3 incarnations to synhronize */
    endp3      = 0, /* to signal demise of p3 */
    endp4      = 0, /* to signal demise of p4 */
    endp7      = 0, /* to signal demise of p7 */
    endcreate  = 0, /* for a p7 leaf to signal its creation */
    blkleaves  = 0, /* for a p7 leaf to signal its creation */
    blkp7      = 0, /* to block p7 */
    blkp7child = 0; /* to block p7's children */

state_t p2state, p3state, p4state, p5state, p6state;
state_t p7rootstate, child1state, child2state;
state_t gchild1state, gchild2state, gchild3state, gchild4state;

/* trap states for p4 */
state_t pstat_n, mstat_n, sstat_n, pstat_o, mstat_o, sstat_o;

int p1p2synch = 0; /* to check on p1/p2 synchronization */

int p7inc;     /* p7's incarnation number */
int p3inc = 1; /* p3 incarnation number */

int          creation      = 0; /* return code for SYSCALL invocation */
memaddr *    p4MemLocation = (memaddr *)0x34;
unsigned int p4Stack;

pid_t p3pid;
pid_t testpid;
pid_t p7pid;
pid_t leaf1pid, leaf2pid, leaf3pid, leaf4pid;

void p2(), p3(), p4(), p4a(), p4b(), p5(), p6(), p6a();
void p7root(), child1(), child2(), p7leaf();

unsigned int set_sp_pc_status(state_t *s, state_t *copy, unsigned int pc) {
    STST(s);

#ifdef TARGET_UMPS
    s->reg_sp = copy->reg_sp - FRAME_SIZE;
    s->pc_epc = pc;
    s->status = STATUS_ALL_INT_ENABLE(s->status);
    return s->reg_sp;
#endif

#ifdef TARGET_UARM
    s->sp   = copy->sp - FRAME_SIZE;
    s->pc   = pc;
    s->cpsr = STATUS_ALL_INT_ENABLE(s->cpsr);
    return s->sp;
#endif
}


/* a procedure to print on terminal 0 */
void print(char *msg) {
    unsigned int command;
    char *       s    = msg;
    devregtr *   base = (devregtr *)DEV_REG_ADDR(IL_TERMINAL, 0);     // (devregtr *)(TERM0ADDR);
    devregtr     status;

    SYSCALL(PASSEREN, (int)&term_mut, 0, 0); /* get term_mut lock */

    while (*s != '\0') {
        /* Put "transmit char" command+char in term0 register (3rd word). This
                 actually starts the operation on the device! */
        command = PRINTCHR | (((devregtr)*s) << BYTELEN);

        /* Wait for I/O completion (SYS8) */
        status = SYSCALL(WAITIO, command, (int)base, FALSE);

        /*		PANIC(); */

        if ((status & TERMSTATMASK) != TRANSM)
            PANIC();

        if (((status & TERMCHARMASK) >> BYTELEN) != ((unsigned) *s))
            PANIC();

        s++;
    }

    SYSCALL(VERHOGEN, (int)&term_mut, 0, 0); /* release term_mut */
}


/*                                                                   */
/*                 p1 -- the root process                            */
/*                                                                   */
void test() {

    SYSCALL(VERHOGEN, (int)&testsem, 0, 0); /* V(testsem)   */

    if (testsem != 1) {
        print("error: p1 v(testsem) with no effects\n");
        PANIC();
    }

    print("p1 v(testsem)\n");

    /* set up states of the other processes */

    /* set up p2's state */
    set_sp_pc_status(&p2state, &p2state, (unsigned int)p2);

    /* Set up p3's state */
    set_sp_pc_status(&p3state, &p2state, (unsigned int)p3);

    /* Set up p4's state */
    p4Stack = set_sp_pc_status(&p4state, &p3state, (unsigned int)p4);

    /* Set up p5's state */
    set_sp_pc_status(&p5state, &p4state, (unsigned int)p5);

    /* Set up p6's state */
    set_sp_pc_status(&p6state, &p5state, (unsigned int)p6);

    /* Set up p7's state */
    set_sp_pc_status(&p7rootstate, &p6state, (unsigned int)p7root);

    /* Set up p7 children's state */
    set_sp_pc_status(&child1state, &p7rootstate, (unsigned int)child1);
    set_sp_pc_status(&child2state, &child1state, (unsigned int)child2);

    /* Set up p7 grandchildren's state */
    set_sp_pc_status(&gchild1state, &child2state, (unsigned int)p7leaf);
    set_sp_pc_status(&gchild2state, &gchild1state, (unsigned int)p7leaf);
    set_sp_pc_status(&gchild3state, &gchild2state, (unsigned int)p7leaf);
    set_sp_pc_status(&gchild4state, &gchild3state, (unsigned int)p7leaf);

    /* create process p2 */
    SYSCALL(CREATEPROCESS, (int)&p2state, DEFAULT_PRIORITY, 0); /* start p2     */
    print("p2 was started\n");

    SYSCALL(VERHOGEN, (int)&startp2, 0, 0); /* V(startp2)   */

    /* P1 blocks until p2 finishes and Vs endp2 */
    SYSCALL(PASSEREN, (int)&endp2, 0, 0); /* P(endp2)     */
    print("p1 knows p2 ended\n");

    /* make sure we really blocked */
    if (p1p2synch == 0)
        print("error: p1/p2 synchronization bad\n");

    SYSCALL(CREATEPROCESS, (int)&p3state, DEFAULT_PRIORITY, (int)&p3pid);     //

    SYSCALL(PASSEREN, (int)&endp3, 0, 0);
    print("p1 knows p3 ended\n");

    SYSCALL(CREATEPROCESS, (int)&p4state, DEFAULT_PRIORITY, 0);     // start p4

    SYSCALL(PASSEREN, (int)&endp4, 0, 0);
    print("p1 knows p4 ended\n");

    SYSCALL(CREATEPROCESS, (int)&p5state, DEFAULT_PRIORITY, 0); /* start p5		*/

    SYSCALL(CREATEPROCESS, (int)&p6state, DEFAULT_PRIORITY, 0); /* start p6		*/

    SYSCALL(VERHOGEN, (int)&blkp7, 0, 0);

    /* now for a more rigorous check of process termination */
    for (p7inc = 0; p7inc < 4; p7inc++) {
        SYSCALL(PASSEREN, (int)&blkp7, 0, 0);
        blkp7child = 0;
        blkleaves  = 0;

        creation = SYSCALL(CREATEPROCESS, (int)&p7rootstate, DEFAULT_PRIORITY, (int)&p7pid);

        if (creation == ERROR) {
            print("error in process creation\n");
            PANIC();
        }

        SYSCALL(PASSEREN, (int)&endp7, 0, 0);
        SYSCALL(TERMINATEPROCESS, (int)p7pid, 0, 0);

        SYSCALL(VERHOGEN, (int)&blkp7, 0, 0);
        SYSCALL(VERHOGEN, (int)&blkp7child, 0, 0);
        SYSCALL(VERHOGEN, (int)&blkp7child, 0, 0);
    }

    print("\n");

    print("p1 finishes OK -- TTFN\n");
    *((memaddr *)BADADDR) = 0; /* terminate p1 */

    /* should not reach this point, since p1 just got a program trap */
    print("error: p1 still alive after progtrap & no trap vector\n");
    PANIC(); /* PANIC !!!     */
}


/* p2 -- semaphore and cputime-SYS test process */
void p2() {
    int   i;                          /* just to waste time  */
    cpu_t now1, now2;                 /* times of day        */
    cpu_t user_t1, user_t2;           /* user time used       */
    cpu_t kernel_t1, kernel_t2;       /* kernel time used       */
    cpu_t wallclock_t1, wallclock_t2; /* wallclock time used       */

    /* startp2 is initialized to 0. p1 Vs it then waits for p2 termination */
    SYSCALL(PASSEREN, (int)&startp2, 0, 0); /* P(startp2)   */

    print("p2 starts\n");

    /* initialize all semaphores in the s[] array */
    for (i = 0; i <= MAXSEM; i++)
        s[i] = 0;

    /* V, then P, all of the semaphores in the s[] array */
    for (i = 0; i <= MAXSEM; i++) {
        SYSCALL(VERHOGEN, (int)&s[i], 0, 0); /* V(S[I]) */
        SYSCALL(PASSEREN, (int)&s[i], 0, 0); /* P(S[I]) */
        if (s[i] != 0)
            print("error: p2 bad v/p pairs\n");
    }

    print("p2 v/p pairs successfully\n");

    /* test of SYS6 */

    now1 = getTODLO();                                                       /* time of day   */
    SYSCALL(GETCPUTIME, (int)&user_t1, (int)&kernel_t1, (int)&wallclock_t1); /* CPU time used */

    /* delay for some time */
    for (i = 1; i < LOOPNUM; i++)
        ;

    SYSCALL(GETCPUTIME, (int)&user_t2, (int)&kernel_t2, (int)&wallclock_t2); /* CPU time used */
    now2 = getTODLO();                                                       /* time of day  */

    if (((user_t2 - user_t1) >= (kernel_t2 - kernel_t1)) && ((wallclock_t2 - wallclock_t1) >= (user_t2 - user_t1)) &&
        ((now2 - now1) >= (wallclock_t2 - wallclock_t1)) && ((user_t2 - user_t1) >= MINLOOPTIME)) {
        print("p2 (semaphores and time check) is OK\n");
    } else {
        if ((user_t2 - user_t1) < (kernel_t2 - kernel_t1))
            print("warning: here kernel time should be less than user time\n");
        if ((wallclock_t2 - wallclock_t1) < (user_t2 - user_t1))
            print("error: more cpu time than wallclock time\n");
        if ((now2 - now1) < (wallclock_t2 - wallclock_t1))
            print("error: more wallclock time than real time\n");
        if ((user_t2 - user_t1) < MINLOOPTIME)
            print("error: not enough cpu time went by\n");
        print("p2 blew it!\n");
    }

    p1p2synch = 1; /* p1 will check this */

    SYSCALL(VERHOGEN, (int)&endp2, 0, 0); /* V(endp2)     */

    SYSCALL(TERMINATEPROCESS, 0, 0, 0); /* terminate p2 */

    /* just did a SYS2, so should not get to this point */
    print("error: p2 didn't terminate\n");
    PANIC(); /* PANIC! */
}


/* p3 -- termination test process and getpid test */
void p3() {
    pid_t pid;
    pid_t p32id;

    switch (p3inc) {
        case 1:
            print("first incarnation of p3 starts\n");
            SYSCALL(GETPID, (int)&pid, 0, 0);
            if (p3pid != pid) {
                print("error: createprocess or getpid are wrong\n");
                PANIC();
            }
            p3inc++;
            break;
        case 2:
            print("second incarnation of p3 starts\n");
            SYSCALL(GETPID, 0, (int)&pid, 0);
            if (p3pid != pid) {
                print("error: createprocess or getppid are wrong\n");
                PANIC();
            }

            break;
    }

    SYSCALL(VERHOGEN, (int)&synp3, 0, 0); /* V(synp3)     */

    /* first incarnation made blkp3=0, the second is blocked (blkp3 become -1) */
    SYSCALL(PASSEREN, (int)&blkp3, 0, 0); /* P(blkp3)     */

    SYSCALL(PASSEREN, (int)&synp3, 0, 0); /* P(synp3)     */

    /* start another incarnation of p3 running, and wait for  */
    /* a V(synp3). the new process will block at the P(blkp3),*/
    /* and eventually, the parent p3 will terminate, killing  */
    /* off both p3's.                                         */

    SP(p3state) = SP(p3state) - FRAME_SIZE; /* give another page  */

    print("p3 create a new p3\n");
    SYSCALL(CREATEPROCESS, (int)&p3state, DEFAULT_PRIORITY, (int)&p32id); /* start a new p3    */

    SYSCALL(PASSEREN, (int)&synp3, 0, 0); /* wait for it       */
    print("p3 termination of the child\n");
    if (((int) SYSCALL(TERMINATEPROCESS, (int)p32id, 0, 0)) < 0) { /* terminate p3      */
        print("error: terminate process is wrong\n");
        PANIC();
    }

    print("p3 is OK\n");

    SYSCALL(VERHOGEN, (int)&endp3, 0, 0); /* V(endp3)          */

    print("p3 termination after the child\n");

    SYSCALL(TERMINATEPROCESS, 0, 0, 0); /* terminate p3      */

    /* just did a SYS2, so should not get to this point */
    print("error: p3 didn't terminate\n");
    PANIC(); /* PANIC            */
}


/* p4's program trap handler */
void p4prog() {
    unsigned int exeCode = CAUSE_CODE(pstat_o);     // pstat_o.cause;

    switch (exeCode) {
        case EXC_BUSINVFETCH:
            print("pgmTrapHandler - Access non-existent memory\n");
            PC(pstat_o) = (memaddr)p4a; /* Continue with p4a() */
            break;

        default:
            print("pgmTrapHandler - other program trap\n");
    }

    LDST(&pstat_o); /* "return" to old area (that changed meanwhile) */
}

/* p4's memory management (tlb) trap handler */
/* void p4mm(unsigned int cause) { */
void p4mm() {
    print("memory management (tlb) trap\n");
    // mstat_o.status = mstat_o.status & 0xFFFFFFF0; /* user mode on */
    VM(mstat_o) &= VMOFF;               /* disable VM */
    PC(mstat_o) = (memaddr)p4b;         /* return to p4b */
    SP(mstat_o) = p4Stack - FRAME_SIZE; /* Start with a fresh stack */

    /* this is made to p4b(). printed in this point because in p4b the kernel mode is off */
    print("p4 - try call sys13 to verify pass up\n");

    LDST(&mstat_o);
}

/* p4's SYS/BK trap handler */
/* void p4sys(unsigned int cause) { */
void p4sys() {
    if (REG0(sstat_o) != 13) {
        print("Not the right syscall code!\n");
        PANIC();
    } else {
        print("Custom system call handler called successfully; continuing...\n");
        LDST(&sstat_o);
    }
}

/* p4 -- SYS5 test process */
void p4() {
    print("p4 starts\n");

    /* set up higher level TRAP handlers (new areas) */
    STST(&pstat_n);                /* pgmtrap new area */
    PC(pstat_n) = (memaddr)p4prog; /* pgmtrap exceptions */

    STST(&mstat_n);              /* tlb new area */
    PC(mstat_n) = (memaddr)p4mm; /* tlb exceptions */

    STST(&sstat_n);               /* sys/bk new area */
    PC(sstat_n) = (memaddr)p4sys; /* sys/bk exceptions */

    /* trap handlers should operate in complete mutex: no interrupts on */
    /* this because they must restart using some BIOS area */

    /* specify trap vectors */
    SYSCALL(SPECPASSUP, 2, (int)&pstat_o, (int)&pstat_n);

    SYSCALL(SPECPASSUP, 1, (int)&mstat_o, (int)&mstat_n);

    SYSCALL(SPECPASSUP, 0, (int)&sstat_o, (int)&sstat_n);

    print("p4 - try to cause a pgm trap access some non-existent memory\n");     // TODO: Restore
    /* to cause a pgm trap access some non-existent memory */
    *p4MemLocation = *p4MemLocation + 1; /* Should cause a program trap */
}

void p4a() {
    unsigned int p4Status;

    print("p4a - try to generate a TLB exception\n");

/* generate a TLB exception by turning on VM without setting up the
         seg tables */
#ifdef TARGET_UMPS
    p4Status = getSTATUS();
    p4Status |= VMON;
    setSTATUS(p4Status);
#elif defined TARGET_UARM
    p4Status = getCONTROL();
    p4Status |= VMON;
    setCONTROL(p4Status);
#endif
}

/* second part of p4 - should be entered in user mode */
void p4b() {
    print("p4b - Invoking custom system call 13\n");
    SYSCALL(13, 0, 0, 0);

    SYSCALL(VERHOGEN, (int)&endp4, 0, 0); /* V(endp4) */

    print("p4 - try to redefine PGMVECT, it will cause p4 termination\n");
    /* should cause a termination       */
    /* since this has already been      */
    /* done for PROGTRAPs               */
    if (SYSCALL(SPECPASSUP, 2, (int)&pstat_o, (int)&pstat_n) == 0) {
        print("error: double SPECPASSUP should not succeed\n");
        PANIC();
    }

    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
    /* should have terminated, so should not get to this point */
    print("error: p4 didn't terminate\n");
    PANIC(); /* PANIC            */
}


/*p5 -- high level syscall without initializing trap vector*/
void p5() {
    print("p5 starts (and hopefully dies)\n");

    SYSCALL(13, 0, 0, 0); /* should cause termination because p5 has no
                                                                                                   trap vector */

    print("error: p5 alive after SYS13() with no trap vector\n");

    PANIC();
}

/*p6 -- program trap without initializing passup vector*/
void p6() {
    print("p6 starts (and hopefully dies)\n");

    *((memaddr *)BADADDR) = 0;

    print("error: p6 alive after program trap with no trap vector\n");
    PANIC();
}


/* p7root -- test of termination of subtree of processes              */
/* create a subtree of processes, wait for the leaves to block, signal*/
/* the root process, and then terminate                               */
void p7root() {
    int i;

    print("p7root starts\n");

    SYSCALL(CREATEPROCESS, (int)&child1state, DEFAULT_PRIORITY, 0);
    SYSCALL(CREATEPROCESS, (int)&child2state, DEFAULT_PRIORITY, 0);

    for (i = 0; i < NOLEAVES; i++)
        SYSCALL(PASSEREN, (int)&endcreate, 0, 0);

    print("Leaves created, now terminating...\n");

    SYSCALL(TERMINATEPROCESS, (int)leaf1pid, 0, 0);
    SYSCALL(TERMINATEPROCESS, (int)leaf2pid, 0, 0);
    SYSCALL(TERMINATEPROCESS, (int)leaf3pid, 0, 0);
    SYSCALL(TERMINATEPROCESS, (int)leaf4pid, 0, 0);

    for (i = 0; i < NOLEAVES; i++)
        SYSCALL(VERHOGEN, (int)&blkleaves, 0, 0);

    SYSCALL(VERHOGEN, (int)&endp7, 0, 0);

    SYSCALL(PASSEREN, (int)&blkp7, 0, 0);

    print("Error: p7root should not reach here!\n");
    PANIC();
}

/*child1 & child2 -- create two sub-processes each*/

void child1() {
    print("child1 starts\n");

    SYSCALL(CREATEPROCESS, (int)&gchild1state, DEFAULT_PRIORITY, (int)&leaf1pid);

    SYSCALL(CREATEPROCESS, (int)&gchild2state, DEFAULT_PRIORITY, (int)&leaf2pid);

    SYSCALL(PASSEREN, (int)&blkp7child, 0, 0);

    print("error: p7 child was not killed with father\n");
    PANIC();
}

void child2() {
    print("child2 starts\n");

    SYSCALL(CREATEPROCESS, (int)&gchild3state, DEFAULT_PRIORITY, (int)&leaf3pid);

    SYSCALL(CREATEPROCESS, (int)&gchild4state, DEFAULT_PRIORITY, (int)&leaf4pid);

    SYSCALL(PASSEREN, (int)&blkp7child, 0, 0);

    print("error: p7 child was not killed with father\n");
    PANIC();
}

/*p7leaf -- code for leaf processes*/

void p7leaf() {
    print("leaf process starts\n");

    SYSCALL(VERHOGEN, (int)&endcreate, 0, 0);

    SYSCALL(PASSEREN, (int)&blkleaves, 0, 0);

    print("error: p7 grandchild was not killed with father\n");
    PANIC();
}
