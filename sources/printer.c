#include <system.h>
#include <printer.h>

#define STATUS_READY        1U
#define STATUS_BUSY         3U

#define CMD_ACK             1U
#define CMD_PRINT           2U

static dtpreg_t *printer0 = (dtpreg_t *) DEV_REG_ADDR(IL_PRINTER, 0);

int printer_putchar(const char c) {
    unsigned status;

    if (STATUS_READY != printer0->status) {
        return -1;
    }

    printer0->data0 = c;
    printer0->command = CMD_PRINT;

    while ((status = printer0->status) == STATUS_BUSY);

    printer0->command = CMD_ACK;
    return (STATUS_READY == status) ? 0 : -1;
}

void printer_puts(const char *s) {
    char c;

    while ((c = *s++)) {
        printer_putchar(c);
    }
}
