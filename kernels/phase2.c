#include <core.h>
#include <test2.h>
#include <scheduler.h>

int main(void) {
   core_boot();

   scheduler_scheduleWith(test, 1, true);

   scheduler_dispatch();
   unreachable();
}
