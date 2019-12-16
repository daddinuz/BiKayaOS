#include <bikaya/printer.h>
#include <bikaya/term.h>

int main(void) {
    int c;

    while (1) {
        term_puts(">>> ");

        for (c = term_getchar(); c >= 0 && c != '\n'; c = term_getchar()) {
            printer_putchar(c);
        }

        printer_putchar('\n');
    }
}
