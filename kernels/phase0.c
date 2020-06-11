#include <primitive_types.h>
#include <helpers.h>
#include <printer.h>
#include <term.h>
#include <core.h>
#include <scheduler.h>

static bool put_char(const char c) {
    return term_putchar(0, c);
}

static bool put_str(const char *const s) {
    return term_puts(0, s);
}

static bool get_char(char *c) {
    return term_getchar(0, c);
}

static void loop(void) { 
    u32 n;
    char c;
    bool r;

    while (true) {
        put_str(">>> ");

        n = 0;
        while ((r = get_char(&c)) && c != '\n' && (r = printer_putchar(0, c))) {
            ++n;
        }

        printer_putchar(0, '\n');
        u32_to_base10(put_char, n);
        put_str((r) ? " characters correctly transmitted.\n" :
                      " characters transmitted, errors occurred.\n");
    }

    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

int main(void) {
    core_boot();

    scheduler_schedule(loop, 1);
    scheduler_dispatch();
}
