#include <assertions.h>
#include <const_bikaya.h>
#include <core.h>
#include <printer.h>

// NOTE: keep this portion of code free of arch-specified code!

// High level states of a printer device.
enum PrinterStatus {
    PRINTER_STATUS_ERR,
    PRINTER_STATUS_BUSY,
    PRINTER_STATUS_READY,
    PRINTER_STATUS_ABSENT,
};

static enum PrinterStatus transmit(unsigned handle, char character);

bool printer_putchar(const unsigned handle, const char character) {
    debug_assert(handle < MACHINE_DEVICE_PRINTER_NO);

    switch(transmit(handle, character)) {
        case PRINTER_STATUS_READY:
            return true;

        default:
            return false;
    }
}

usize printer_puts(const unsigned handle, const char *str) {
    usize i = 0;

    if (NULL != str) {
        while (*str && printer_putchar(handle, *(str++))) {
            ++i;
        }
    }

    return i;
}

/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!    ATTENTION    !!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * The code below contains arch-dependant code!
 *
 * The main target of BiKaya is portability between architectures so we defined
 * an interface (printer.h) which is independent from the underlying machine and
 * can be easily implemented for other architectures, but since right now we
 * only have to support uARM and uMPS, which actually work the same regarding
 * device handling (except some architecture-specific consts), we are going to
 * implement the interface exposed in printer.h in the most straightforward way,
 * aware that if support for other architectures will be required the following
 * implementation must be refactored.
 */

#if !(defined(TARGET_UARM) || defined(TARGET_UMPS))
#error "Unknown target architecture"
#endif

#define RAW_STATUS_ERR       4U
#define RAW_STATUS_BUSY      3U
#define RAW_STATUS_READY     1U
#define RAW_STATUS_ABSENT    0U

#define CMD_ACK              1U
#define CMD_PRINT            2U

static enum PrinterStatus decodeStatus(unsigned rawStatus);

static enum PrinterStatus transmit(const unsigned handle, const char character) {
    debug_assert(handle < MACHINE_DEVICE_PRINTER_NO);
    dtpreg_t *const printerRegister = (dtpreg_t *) DEV_REG_ADDR(INTERRUPT_LINE_PRINTER, handle);

    printerRegister->data0 = character;
    return decodeStatus((unsigned) SYSCALL(WAITIO, CMD_PRINT, (memaddr) printerRegister, 0));
}

static enum PrinterStatus decodeStatus(const unsigned rawStatus) {
    switch(rawStatus) {
        case RAW_STATUS_ERR:
            return PRINTER_STATUS_ERR;

        case RAW_STATUS_BUSY:
            return PRINTER_STATUS_BUSY;

        case RAW_STATUS_READY:
            return PRINTER_STATUS_READY;

        case RAW_STATUS_ABSENT:
            return PRINTER_STATUS_ABSENT;

        default:
            unreachable();
    }
}
