#include <bikaya/primitive_types.h>
#include <bikaya/helpers.h>
#include <bikaya/printer.h>
#include <bikaya/term.h>

int main(void) {
    u32 n;
    char c;
    bool s;

    while (true) {
        term_puts(">>> ");

        n = 0;
        while ((s = term_getchar(&c)) && c != '\n' && (s = printer_putchar(c))) {
            ++n;
        }

        printer_putchar('\n');
        u32_to_base10(term_putchar, n);
        term_puts((s) ? " characters correctly transmitted.\n" :
                        " characters transmitted, errors occurred.\n");
    }
}
