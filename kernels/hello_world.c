#include <core.h>
#include <scheduler.h>

void io(void) {
    const char str[] = "Hello world!\n";

    for (const char *cur = str; *cur; cur++) {
        unsigned c = *cur;
        c <<= 8;
        c |= 2;
        SYSCALL(WAITIO, c, DEV_REG_ADDR(IL_TERMINAL, 0), 0);
    }

    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

int main(void) {
   core_boot();

   scheduler_scheduleWith(io, 1, true);

   scheduler_dispatch();
   unreachable();
}
