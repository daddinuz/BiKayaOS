#pragma once

#include <primitive_types.h>

/**
 * Prints a single character.
 * The character is intended to be a valid ASCII character (range [0, 127]),
 * internally the character is converted to an unsigned char.
 *
 * @attention Passing an invalid handle is a checked runtime error.
 * @attention Passing a character with value outside range [0, 127] is UB.
 *
 * @param handle The terminal handle.
 * @param character The character to be transmitted.
 * @return On success true, false otherwise.
 */
extern bool term_putchar(unsigned handle, char character);

/**
 * Prints a string.
 * The string is intended to be composed of valid ASCII characters (range [0, 127]),
 * internally those characters are converted to unsigned char(s).
 * If the string is NULL this function will result in noop returning 0.
 *
 * @attention passing an invalid handle is a checked runtime error.
 * @attention The string must be NULL terminated; passing characters with values
 * outside range [0, 127] is UB.
 *
 * @param handle The terminal handle.
 * @param str The string to be transmitted.
 * @return The number of characters correctly transmitted.
 */
extern usize term_puts(unsigned handle, const char *str);

/**
 * Reads a character from the terminal and stores it into the specified buffer.
 * This function is blocking and will wait for user input.
 * If the buffer is NULL the character read will be discarded.
 *
 * @attention passing an invalid handle is a checked runtime error.
 * @attention buf will be modified only if it is not NULL and after a
 * successful read, in all other cases it will be left untouched.
 *
 * @param handle The terminal handle.
 * @param buf The buffer where the character will be put after a successful read.
 * @return On success true, false otherwise.
 */
extern bool term_getchar(unsigned handle, char *buf);

/**
 * Reads characters from the terminal and stores them into buffer
 * until (n-1) characters have been read or either termination is reached,
 * whichever happens first.
 * This function is blocking and will wait for user input.
 * If the buffer is NULL the characters read will be discarded.
 *
 * Note: Calling this function with
 *  - n == 0: Will cause this function to return 0.
 *            No reads will be performed and the buffer will be left untouched.
 *  - n == 1: Will cause this function to return 0.
 *            No reads will be performed and the buffer will be null-terminated.
 *
 * @attention passing an invalid handle is a checked runtime error.
 * @attention buf will be modified only if it is not NULL and after a
 * successful read, in all other cases it will be left untouched.
 *
 * @attention if p is NULL, '\n' will be used as a termination character.
 *
 * @param handle The terminal handle.
 * @param p The predicate used to determinate if termination is reached, which
 *          means that p returns false as long as the termination character has
 *          not been reached, true instead.
 * @param buf The buffer where the characters will be put after a successful read.
 * @param n The size of the buffer.
 * @return The number of characters read (in range [0, n-1]).
 */
extern usize term_gets(unsigned handle, bool (*p)(char), char *buf, usize n);
