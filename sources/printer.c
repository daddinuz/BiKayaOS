#include <system.h>
#include <printer.h>

#define STATUS_READY        1U
#define STATUS_BUSY         3U

#define CMD_ACK             1U
#define CMD_PRINT           2U

static dtpreg_t *printer0 = (dtpreg_t *) DEV_REG_ADDR(IL_PRINTER, 0);

bool printer_putchar(const char character) {
    unsigned status;

    if (STATUS_BUSY == printer0->status) {
        return false;
    }

    printer0->data0 = character;
    printer0->command = CMD_PRINT;

    while ((status = printer0->status) == STATUS_BUSY);

    printer0->command = CMD_ACK;
    return STATUS_READY == status;
}

usize printer_puts(const char *str) {
    usize i = 0;

    if (NULL != str) {
        while (*str && printer_putchar(*(str++))) {
            ++i;
        }
    }

    return i;
}
