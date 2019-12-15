#include <system.h>
#include <term.h>

#define STAT_READY          1
#define STAT_BUSY           3
#define STAT_OK             5

#define CMD_ACK             1
#define CMD_TRANSMIT        2
#define CMD_RECEIVE         2

#define CHAR_OFFSET         8
#define TERM_STATUS_MASK    0xFF

static termreg_t *term0 = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);

static unsigned int tx_transm_status(const termreg_t *const tp) {
    return tp->transm_status & TERM_STATUS_MASK;
}

static unsigned int tx_recv_status(const termreg_t *const tp) {
    return tp->recv_status & TERM_STATUS_MASK;
}

int term_putchar(const char c) {
    unsigned int stat;

    stat = tx_transm_status(term0);
    if (STAT_READY != stat) {
        return -1;
    }

    term0->transm_command = ((c << CHAR_OFFSET) | CMD_TRANSMIT);
    while ((stat = tx_transm_status(term0)) == STAT_BUSY);

    term0->transm_command = CMD_ACK;
    return (STAT_OK == stat) ? 0 : -1;
}

void term_puts(const char *str) {
    char c;

    while ((c = *str++)) {
        term_putchar(c);
    }
}

int term_getchar(void) {
    unsigned int stat;
    int c = -1;

    stat = tx_recv_status(term0);
    if (STAT_READY != stat) {
        return -1;
    }

    term0->recv_command = CMD_RECEIVE;
    while ((stat = tx_recv_status(term0)) == STAT_BUSY);

    if (STAT_OK == stat) {
        // Receive command.
        //
        //  | OPAQ | OPAQ | CHAR | STAT |
        // 31     23     15      7      0
        //
        // 8 is the size in bits of the status field.
        // 0xFF is the mask used to extract the first byte of a word.
        c = (term0->recv_status >> 8) & 0xFF;
    }

    term0->recv_command = CMD_ACK;
    return (STAT_OK == stat) ? c : -1;
}
