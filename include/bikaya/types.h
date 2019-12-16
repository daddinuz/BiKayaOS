#pragma once

#include <bikaya/static_assert.h>

// boolean type and consts
#define bool _Bool
#define true ((bool) 1)
#define false ((bool) 0)

// 8-bits integer types
static_assert(sizeof(char) == 1, "sizeof(char) != 1");
typedef unsigned char u8;
typedef signed char i8;

// 16-bits integer types
static_assert(sizeof(short) == 2, "sizeof(short) != 2");
typedef unsigned short u16;
typedef signed short i16;

// 32-bits integer types
static_assert(sizeof(int) == 4, "sizeof(int) != 4");
typedef unsigned int u32;
typedef signed int i32;

// size types (biggest width available)
static_assert(sizeof(long long) >= sizeof(int), "sizeof(long long) < sizeof(int)");
typedef unsigned long long usize;
typedef signed long long isize;

// 32-bits floating type
static_assert(sizeof(float) == 4, "sizeof(float) != 4");
typedef float f32;

// 64-bits floating type
static_assert(sizeof(double) == 8, "sizeof(double) != 8");
typedef double f64;
