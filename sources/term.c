#include <system.h>
#include <term.h>

#define STATUS_READY        1U
#define STATUS_BUSY         3U
#define STATUS_OK           5U

#define CMD_ACK             1U
#define CMD_TRANSMIT        2U
#define CMD_RECEIVE         2U

#define CHAR_OFFSET         8U
#define TERM_STATUS_MASK    0xFFU

static termreg_t *term0 = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);

static inline unsigned term_transmit_status(const termreg_t *const tp) {
    return tp->transm_status & TERM_STATUS_MASK;
}

static inline unsigned term_receive_status(const termreg_t *const tp) {
    return tp->recv_status & TERM_STATUS_MASK;
}

int term_putchar(const char c) {
    unsigned status = term_transmit_status(term0);

    if (STATUS_READY != status) {
        return -1;
    }

    term0->transm_command = ((((unsigned) c) << CHAR_OFFSET) | CMD_TRANSMIT);
    while ((status = term_transmit_status(term0)) == STATUS_BUSY);

    term0->transm_command = CMD_ACK;
    return (STATUS_OK == status) ? 0 : -1;
}

void term_puts(const char *str) {
    char c;

    while ((c = *str++)) {
        term_putchar(c);
    }
}

int term_getchar(void) {
    unsigned status = term_receive_status(term0);

    if (STATUS_READY != status) {
        return -1;
    }

    term0->recv_command = CMD_RECEIVE;
    while ((status = term_receive_status(term0)) == STATUS_BUSY);

    if (STATUS_OK == status) {
        // Receive command.
        //
        //  | OPAQUE | OPAQUE |  CHAR  | STATUS |
        // 31       23       15        7        0
        //
        // 8 is the size in bits of the status field.
        // 0xFF is the mask used to extract the first byte of a word.
        int c = (int) ((term0->recv_status >> 8U) & 0xFFU);

        term0->recv_command = CMD_ACK;
        return c;
    } else {
        term0->recv_command = CMD_ACK;
        return -1;
    }
}
