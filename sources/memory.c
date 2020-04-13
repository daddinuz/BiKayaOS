#include <assertions.h>
#include <memory.h>

void memclr(void *const ptr, usize len) {
    debug_assert(NULL != ptr);

    u8 *p = ptr;
    while (len--) {
        *(p++) = 0;
    }
}

void memdup(void *const destination, const void *const source, usize len) {
    debug_assert(NULL != destination);
    debug_assert(NULL != source);

    u8 *d = destination;
    const u8 *s = source;
    while (len--) {
        *(d++) = *(s++);
    }
}
