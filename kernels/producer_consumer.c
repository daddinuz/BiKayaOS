#include <core.h>
#include <term.h>
#include <scheduler.h>

int data = -1;
int ok2Read = 0;
int ok2Write = 1;

void producer(void) {
    for(int j = 0; j < 10; ++j) {
        SYSCALL(PASSEREN, (memaddr) &ok2Write, 0, 0);

        data += 1;

        SYSCALL(VERHOGEN, (memaddr) &ok2Read, 0, 0);
    }

    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

void consumer(void) {
    int localData = -1;

    do {
        SYSCALL(PASSEREN, (memaddr) &ok2Read, 0, 0);

        localData = data;
        term_putchar(0, '0' + localData);

        SYSCALL(VERHOGEN, (memaddr) &ok2Write, 0, 0);
    } while(localData < 9);

    term_putchar(0, '\n');
    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

int main(void) {
    core_boot();

    scheduler_schedule(producer, 1);
    scheduler_schedule(consumer, 2);

    scheduler_dispatch();
    unreachable();
}
