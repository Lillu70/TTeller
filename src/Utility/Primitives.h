
#pragma once

#define Array_Lenght(array) sizeof(array) / sizeof(array[0])

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float f32;
typedef double f64;

typedef i32 b32;

// CONSIDER: Use #define or constexpr here.

static constexpr f64 PI = 3.141592653589793;
static constexpr f32 PI32 = f32(PI);
static constexpr f32 HALF_PI32 = PI32 / 2;
static constexpr f64 TAU = 6.283185307179586;
static constexpr f32 TAU32 = f32(TAU);
static constexpr u32 DEG_FULL_CIRCLE = 360;
static constexpr u32 DEG_HALF_CIRCLE = DEG_FULL_CIRCLE / 2;
static constexpr f32 F32_MAX = 3.402823466e+38F;
static constexpr f64 F64_MAX = 1.7976931348623157E+308;
static constexpr u32 U32_MAX = 0xFFFFFFFF;
static constexpr i32 I32_MAX = 2147483647;
static constexpr u16 U16_MAX = 0xFFFF;

#define WHITE 0xffffffff
#define BLACK 0xff000000
#define RED 0xffff0000
#define YELLOW 0xffffff00
#define GREEN 0xff00ff00
#define BLUE 0xff0000ff
#define MAGENTA 0xffff00ff

#define KiB 1024
#define MiB KiB * KiB

#include "Vector.h" // v2i, v2u, v2f 

struct Rect
{
	v2f min = {};
	v2f max = {};
};