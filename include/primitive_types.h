#pragma once

// null pointer
#ifndef NULL
#define NULL ((void *) 0)
#endif

// boolean type (since C99)
typedef _Bool bool;

// boolean consts
#define true     1
#define false    0

// 8-bits integer types
typedef unsigned char u8;
typedef signed char i8;

// 16-bits integer types
typedef unsigned short u16;
typedef signed short i16;

// 32-bits integer types
typedef unsigned int u32;
typedef signed int i32;

// size types (biggest width available)
typedef unsigned long long usize;
typedef signed long long isize;

// 32-bits floating type
typedef float f32;

// 64-bits floating type
typedef double f64;
