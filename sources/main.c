#include <bikaya/primitive_types.h>
#include <bikaya/printer.h>
#include <bikaya/term.h>

int main(void) {
    char c;

    while (true) {
        term_puts(">>> ");

        while (term_getchar(&c) && c != '\n') {
            printer_putchar(c);
        }

        printer_putchar('\n');
    }
}
