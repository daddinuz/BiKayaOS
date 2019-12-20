#pragma once

#include <bikaya/primitive_types.h>

/**
 * Takes a number n and converts its digits sequentially into their decimal
 * representation starting from the most significant to the least one, then
 * applies f to each char thus obtained in the same order as long as both
 * f returns true and there are digits left.
 *
 * Note: Internally this function won't allocate any buffer to perform
 *       the conversion instead it converts each digit one by one and applies f
 *       terminating the execution if f returns false.
 *
 * @attention if f is NULL a noop function will be used.
 *
 * @param f The function which consumes each character composing the number
 *          applying its own logic, this function returns true if the execution
 *          must continue false otherwise.
 * @param n The number to be converted.
 * @return true if each sequantial call to f returned true, false otherwise.
 */
bool u32_to_base10(bool (*f)(char), u32 n);
