#include <assertions.h>
#include <const_bikaya.h>
#include <core.h>
#include <term.h>

// NOTE: keep this portion of code free of arch-specific code!

// High level states of a terminal device.
enum TermStatus {
    TERM_STATUS_OK,
    TERM_STATUS_ERR,
    TERM_STATUS_BUSY,
    TERM_STATUS_READY,
    TERM_STATUS_ABSENT,
};

static enum TermStatus transmit(unsigned handle, char c);
static enum TermStatus receive(unsigned handle, char *buf);

static bool isNewline(const char c) {
    return '\n' == c;
}

bool term_putchar(const unsigned handle, const char character) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);

    switch(transmit(handle, character)) {
        case TERM_STATUS_OK:
            /* FALLTHROUGH */
        case TERM_STATUS_READY:
            return true;

        default:
            return false;
    }
}

usize term_puts(const unsigned handle, const char *str) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    usize i = 0;

    if (NULL != str) {
        while (*str && term_putchar(handle, *(str++))) {
            ++i;
        }
    }

    return i;
}

bool term_getchar(const unsigned handle, char *const buf) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);

    switch(receive(handle, buf)) {
        case TERM_STATUS_OK:
            /* FALLTHROUGH */
        case TERM_STATUS_READY:
            return true;

        default:
            return false;
    }
}

usize term_gets(const unsigned handle, bool (*p)(char), char *buf, const usize n) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    usize i = 1;
    bool stop = false;
    p = (NULL == p) ? isNewline : p;

    if (NULL == buf) {
        for (char c = 0; !stop && i < n && term_getchar(handle, &c); ++i, stop = p(c));
    } else {
        for (char c = 0; !stop && i < n && term_getchar(handle, &c); ++i, stop = p(c)) {
            *buf++ = c;
        }

        // if something has been read or the size of the buffer is one then
        // guarantee that the string is null-terminated.
        if (i > 1 || n == 1) {
            *buf = '\0';
        }
    }

    return i - 1;
}

/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!    ATTENTION    !!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * The code below contains arch-dependant code!
 *
 * The main target of BiKaya is portability between architectures so we defined
 * an interface (term.h) which is independent from the underlying machine and
 * can be easily implemented for other architectures, but since right now we
 * only have to support uARM and uMPS, which actually work the same regarding
 * device handling (except some architecture-specific consts), we are going to
 * implement the interface exposed in term.h in the most straightforward way,
 * aware that if support for other architectures will be required the following
 * implementation must be refactored.
 */

#if !(defined(TARGET_UARM) || defined(TARGET_UMPS))
#error "Unknown target architecture"
#endif

#define RAW_STATUS_OK        5U
#define RAW_STATUS_ERR       4U
#define RAW_STATUS_BUSY      3U
#define RAW_STATUS_READY     1U
#define RAW_STATUS_ABSENT    0U

#define CMD_ACK              1U
#define CMD_TRANSMIT         2U
#define CMD_RECEIVE          2U

#define BYTE_OFFSET          8U
#define BYTE_MASK            0xFFU

static enum TermStatus decodeStatus(unsigned rawStatus);

static enum TermStatus transmit(const unsigned handle, const char c) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);

    unsigned command = c;
    command <<= BYTE_OFFSET;
    command |= CMD_TRANSMIT;

    return decodeStatus((unsigned) SYSCALL(WAITIO, command, DEV_REG_ADDR(INTERRUPT_LINE_TERMINAL, handle), 0));
}

static enum TermStatus receive(const unsigned handle, char *const buf) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);

    const unsigned rawStatus = (unsigned) SYSCALL(WAITIO, CMD_RECEIVE, DEV_REG_ADDR(INTERRUPT_LINE_TERMINAL, handle), 1);
    const enum TermStatus status = decodeStatus(rawStatus);

    if (TERM_STATUS_OK == status && NULL != buf) {
        //  | OPAQUE | OPAQUE |  CHAR  | STATUS |
        // 31       23       15        7        0
        *buf = (char) ((rawStatus >> BYTE_OFFSET) & BYTE_MASK);
    }

    return status;
}

static enum TermStatus decodeStatus(const unsigned rawStatus) {
    switch (rawStatus & BYTE_MASK) {
        case RAW_STATUS_OK:
            return TERM_STATUS_OK;

        case RAW_STATUS_ERR:
            return TERM_STATUS_ERR;

        case RAW_STATUS_BUSY:
            return TERM_STATUS_BUSY;

        case RAW_STATUS_READY:
            return TERM_STATUS_READY;

        case RAW_STATUS_ABSENT:
            return TERM_STATUS_ABSENT;

        default:
            unreachable();
    }
}
