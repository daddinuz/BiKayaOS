#pragma once

#include <primitive_types.h>

/**
 * States of a printer device.
 */
enum PrinterStatus {
    PRINTER_STATUS_ERR,
    PRINTER_STATUS_BUSY,
    PRINTER_STATUS_READY,
    PRINTER_STATUS_ABSENT,
};

/**
 * Prints a single character to the default printer.
 * The character is intended to be a valid ASCII character (range [0, 127]),
 * internally the character is converted to an unsigned char.
 *
 * @attention Passing a character with value outside range [0, 127] is UB.
 *
 * @param character The character to be transmitted.
 * @return On success true, false otherwise.
 */
extern bool printer_putchar(unsigned handle, char character);

/**
 * Prints a string to the default printer.
 * The string is intended to be composed of valid ASCII characters (range [0, 127]),
 * internally those characters are converted to unsigned char(s).
 * If the string is NULL this function will result in noop returning 0.
 *
 * @attention The string must be NULL terminated; passing characters with values
 * outside range [0, 127] is UB.
 *
 * @param str The string to be transmitted.
 * @return The number of characters correctly transmitted.
 */
extern usize printer_puts(unsigned handle, const char *str);

/**
 * Gets the status of transmission.
 *
 * @attention passing an invalid handle is a checked runtime error.
 *
 * @param handle The number of printer device.
 * @return The status of transmission.
 */
extern enum PrinterStatus printer_getTransmissionStatus(unsigned handle);

/**
 * Sends ack command to the transmitter of the specified printer.
 *
 * @attention passing an invalid handle is a checked runtime error.
 * 
 * @param handle The number of printer device.
 */
extern void printer_ackTransmission(unsigned handle);
