#include <bikaya/system.h>
#include <bikaya/term.h>

#define STATUS_READY        1U
#define STATUS_BUSY         3U
#define STATUS_OK           5U

#define CMD_ACK             1U
#define CMD_TRANSMIT        2U
#define CMD_RECEIVE         2U

#define BYTE_OFFSET         8U
#define BYTE_MASK           0xFFU

static termreg_t *term0 = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);

static inline unsigned term_transmit_status(const termreg_t *const tp) {
    return tp->transm_status & BYTE_MASK;
}

static inline unsigned term_receive_status(const termreg_t *const tp) {
    return tp->recv_status & BYTE_MASK;
}

bool term_putchar(const char character) {
    unsigned status = term_transmit_status(term0);

    if (STATUS_READY != status) {
        return false;
    }

    term0->transm_command = (((unsigned) character) << BYTE_OFFSET) | CMD_TRANSMIT;
    while ((status = term_transmit_status(term0)) == STATUS_BUSY);

    term0->transm_command = CMD_ACK;
    return STATUS_OK == status;
}

usize term_puts(const char *str) {
    usize i = 0;

    if (NULL != str) {
        while (*str) {
            if (term_putchar(*str++)) {
                ++i;
            } else {
                break;
            }
        }
    }

    return i;
}

bool term_getchar(char *buf) {
    unsigned status = term_receive_status(term0);

    if (STATUS_READY != status) {
        return -1;
    }

    term0->recv_command = CMD_RECEIVE;
    while ((status = term_receive_status(term0)) == STATUS_BUSY);

    if (STATUS_OK == status) {
        if (NULL != buf) {
            // Receive command.
            //
            //  | OPAQUE | OPAQUE |  CHAR  | STATUS |
            // 31       23       15        7        0
            *buf = (char) ((term0->recv_status >> BYTE_OFFSET) & BYTE_MASK);
        }

        term0->recv_command = CMD_ACK;
        return true;
    } else {
        term0->recv_command = CMD_ACK;
        return false;
    }
}

static bool is_newline(const char c) {
    return '\n' == c;
}

usize term_gets(bool (*p)(char), char *buf, const usize n) {
    usize i = 1;
    bool stop = false;
    p = (NULL == p) ? is_newline : p;

    if (NULL == buf) {
        for (char c; !stop && i < n && term_getchar(&c); ++i, stop = p(c));
    } else {
        for (char c; !stop && i < n && term_getchar(&c); ++i, stop = p(c)) {
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
