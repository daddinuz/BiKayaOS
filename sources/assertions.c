#include <term.h>
#include <core.h>
#include <assertions.h>

void __assert(const char *const trace, const char *const assertion, const int c) {
    if (!c) {
        term_puts(0, "At: \"");
        term_puts(0, trace);
        term_puts(0, "\"\nAssertion: `");
        term_puts(0, assertion);
        term_puts(0, "` failed.\n");
        core_panic();
    }
}
