#include <term.h>
#include <system.h>
#include <assertions.h>

void __assert(const char *const trace, const char *const assertion, const int c) {
    if (!c) {
        term_puts("At: \"");
        term_puts(trace);
        term_puts("\"\nAssertion: `");
        term_puts(assertion);
        term_puts("` failed.\n");
        PANIC();
    }
}
