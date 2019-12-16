#pragma once

// static assertion (since C11)
#define static_assert _Static_assert

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

// 32-bits floating type
static_assert(sizeof(float) == 4, "sizeof(float) != 4");
typedef float f32;

// 64-bits floating type
static_assert(sizeof(double) == 8, "sizeof(double) != 8");
typedef double f64;
