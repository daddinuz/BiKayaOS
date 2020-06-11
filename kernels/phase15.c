#include <core.h>
#include <test15.h>
#include <scheduler.h>

int main(void) {
    core_boot();

    scheduler_schedule(test1, 1);
    scheduler_schedule(test2, 1);
    scheduler_schedule(test3, 1);

    scheduler_dispatch();
    unreachable();
}
