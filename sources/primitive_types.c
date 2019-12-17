#include <bikaya/static_assert.h>
#include <bikaya/primitive_types.h>

static_assert(sizeof(u8) == 1, "sizeof(u8) != 1");
static_assert(sizeof(i8) == 1, "sizeof(i8) != 1");

static_assert(sizeof(u16) == 2, "sizeof(u16) != 2");
static_assert(sizeof(i16) == 2, "sizeof(i16) != 2");

static_assert(sizeof(u32) == 4, "sizeof(u32) != 4");
static_assert(sizeof(i32) == 4, "sizeof(i32) != 4");

static_assert(sizeof(usize) >= sizeof(u32), "sizeof(usize) < sizeof(u32)");
static_assert(sizeof(isize) >= sizeof(i32), "sizeof(isize) < sizeof(i32)");

static_assert(sizeof(f32) == 4, "sizeof(f32) != 4");
static_assert(sizeof(f64) == 8, "sizeof(f64) != 8");
