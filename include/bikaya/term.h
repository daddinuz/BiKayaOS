#pragma once

#include <bikaya/primitive_types.h>

/**
 * Prints a single character to the default terminal.
 * The character is intended to be a valid ASCII character (range 0-127),
 * internally the character is converted to an unsigned char.
 *
 * @attention Passing a character with value outside range 0-127 is UB.
 *
 * @param character The character to be transmitted.
 * @return On success true, false otherwise.
 */
extern bool term_putchar(char character);

/**
 * Prints a string to the default terminal.
 * The string is intended to be composed of valid ASCII characters (range 0-127),
 * internally those characters are converted to unsigned char(s).
 * If the string is NULL this function will result in noop returning 0.
 *
 * @attention The string must be NULL terminated; passing characters with values
 * outside range 0-127 is UB.
 *
 * @param str The string to be transmitted.
 * @return the number of characters correctly transmitted.
 */
extern usize term_puts(const char *str);

/**
 * Reads a single character into the specified buffer.
 * The function is blocking and will wait for user input.
 * If the buffer is NULL the character read will be discarded.
 *
 * @attention buf will be modified only after a successful read and only if it
 * is not NULL in all other cases it will be left untouched.
 *
 * @param buf The buffer where the character will be put after a successful read.
 * @return On success true, false otherwise.
 */
extern bool term_getchar(char *buf);

// TODO
// extern usize term_read(char *buf, usize n);
