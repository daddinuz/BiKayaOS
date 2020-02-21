#include <system.h>
#include <helpers.h>

const u32 U32_TEN = 10U;

static bool noop(char c) {
    (void) c;
    return true;
}

bool u32_to_base10(bool (*f)(char), const u32 n) {
    bool r = true;
    f = (NULL == f) ? noop : f;

    if (n > 0) {
        const u32 m = n / U32_TEN;
        if (m > 0) {
            r = u32_to_base10(f, m);
        }
    }

    return r && f('0' + (n % U32_TEN));
}
