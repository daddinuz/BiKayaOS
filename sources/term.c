#include <assertions.h>
#include <core.h>
#include <term.h>

// NOTE: keep this portion of code free of arch-specific code!

static enum TermStatus transmit(unsigned handle, char c);
static enum TermStatus receive(unsigned handle, char *buf);

static bool isNewline(const char c) {
    return '\n' == c;
}

bool term_putchar(const unsigned handle, const char character) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    enum TermStatus status;

    // busy waiting for ready status.
    while ((status = term_getTransmissionStatus(handle)) != TERM_STATUS_READY) {
        if (TERM_STATUS_ABSENT == status) {
            return false;
        }
    }

    // send transmit command.
    status = transmit(handle, character);

    // ack transmission regardless of outcome.
    term_ackTransmission(handle);

    // check if the operation was successful.
    return TERM_STATUS_OK == status;
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
    enum TermStatus status;

    // busy waiting for ready status.
    while ((status = term_getReceptionStatus(handle)) != TERM_STATUS_READY) {
        if (TERM_STATUS_ABSENT == status) {
            return false;
        }
    }

    // send receive command.
    status = receive(handle, buf);

    // ack reception regardless of outcome.
    term_ackReception(handle);

    // check if the operation was successful.
    return TERM_STATUS_OK == status;
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

static termreg_t *getRegister(unsigned handle);
static enum TermStatus decodeStatus(unsigned rawStatus);

enum TermStatus term_getTransmissionStatus(const unsigned handle) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    const termreg_t *const term = getRegister(handle);
    return decodeStatus(term->transm_status);
}

void term_ackTransmission(const unsigned handle) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    termreg_t *const term = getRegister(handle);
    term->transm_command = CMD_ACK;
}

enum TermStatus term_getReceptionStatus(const unsigned handle) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    const termreg_t *const term = getRegister(handle);
    return decodeStatus(term->recv_status);
}

void term_ackReception(const unsigned handle) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    termreg_t *const term = getRegister(handle);
    term->recv_command = CMD_ACK;
}

static termreg_t *getRegister(const unsigned handle) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);
    return (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, handle);
}

static enum TermStatus transmit(const unsigned handle, const char c) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);

    termreg_t *const term = getRegister(handle);
    enum TermStatus status;

    // send char to terminal.
    term->transm_command = (((unsigned) c) << BYTE_OFFSET) | CMD_TRANSMIT;

    // wait for ok or err.
    while ((status = term_getTransmissionStatus(handle)) == TERM_STATUS_BUSY) {}

    return status;
}

static enum TermStatus receive(const unsigned handle, char *const buf) {
    debug_assert(handle < MACHINE_DEVICE_TERMINAL_NO);

    termreg_t *const term = getRegister(handle);
    enum TermStatus status;

    // read char from terminal.
    term->recv_command = CMD_RECEIVE;

    // wait for ok or err.
    while ((status = term_getReceptionStatus(handle)) == TERM_STATUS_BUSY) {}

    if (TERM_STATUS_OK == status && NULL != buf) {
        //  | OPAQUE | OPAQUE |  CHAR  | STATUS |
        // 31       23       15        7        0
        *buf = (char) ((term->recv_status >> BYTE_OFFSET) & BYTE_MASK);
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
