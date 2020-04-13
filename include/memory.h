#pragma once

#include <primitive_types.h>

/**
 * Clears the memory starting from ptr for the following len bytes,
 * setting each of them to zero.
 *
 * @attention (ptr == NULL) is a checked runtime error.
 */
extern void memclr(void *ptr, usize len);

/**
 * Copies memory from source to destination for len bytes.
 *
 * @attentiont memory must NOT overlap!
 *
 * @attention (destination == NULL) is a checked runtime error.
 * @attention (source == NULL) is a checked runtime error.
 */
extern void memdup(void *destination, const void *source, usize len);
