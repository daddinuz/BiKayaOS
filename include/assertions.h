#pragma once

#include <macros.h>

/**
 * NOTE: do not call this function directly, use the `assert` macro instead.
 *
 * Ensures that a given condition `c` occurs, aborting execution otherwise.
 *
 * @param trace The file and line number where the assertion is made.
 * @param assertion The stringified assertion.
 * @param c The condition to test.
 */
extern void __assert(const char *trace, const char *assertion, int c);

/**
 * Macro that aborts execution if condition `c` not occurs.
 *
 * @attention this macro aborts execution when `c` is false even when NDEBUG is defined.
 */
#define assert(c)        __assert(__FILE__ ":" str(__LINE__), str((c)), (c))

/**
 * Macro that aborts execution if condition `c` not occurs (debug-builds only).
 *
 * @attention this macro is noop when NDEBUG is defined.
 */
#ifdef NDEBUG
#define debug_assert(c)
#else
#define debug_assert(c)  assert(c)
#endif

// static assertion (since C11)
#define static_assert    _Static_assert
