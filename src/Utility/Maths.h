
#pragma once

#include "math.h"

#if 1
template<typename T>
static inline T Max(T value, T max)
{
	T result = (value > max)? value : max;
	return result;
}


template<typename T>
static inline T Min(T value, T min)
{
	T result = (value < min)? value : min;
	return result;
}
#endif

static inline f32 Sin(f32 value)
{
	f32 result = sinf(value);
	return result;	
}


static inline f32 Cos(f32 value)
{
	f32 result = cosf(value);
	return result;	
}


static inline f32 Root(f32 value)
{
	f32 result = sqrtf(value);
	return result;
}


static inline f32 Square(f32 value)
{
	f32 result = value * value;
	return result;
}


// Rounds to the nearest whole number towards negative infinity.
static inline f32 Floor(f32 value)
{
	f32 result = f32((i32)value);
	if(result > value)
		result -= 1;

	return result;	
}


static inline f64 Floor(f64 value)
{
	f64 result = f64((i64)value);
	if(result > value)
		result -= 1;

	return result;	
}


static inline v2f Floor(v2f v)
{
	v2f result = { Floor(v.x), Floor(v.y) };
	return result;
}


static inline v2f Trunc(v2f v)
{
	v2f result = { f32(i32(v.x)), f32(i32(v.y)) };
	return result;
}


// Rounds to the nearest whole number towards positive infinity.
static inline f32 Ceil(f32 value)
{
	f32 result = f32((i32)value);
	if(result < value)
		result += 1;
	
	return result;
}


static inline v2f Ceil(v2f v)
{
	v2f result = { Ceil(v.x), Ceil(v.y) };
	return result;
}


static inline f32 Round(f32 real)
{
	f32 result = (f32)((i32)(real + 0.5f));
	return result;
}


static inline v2f Hadamar(v2f a, v2f b)
{
	v2f result = {a.x * b.x, a.y * b.y};
	return result;
}


static inline f32 Componentwise_Mult(v2f v)
{
	f32 result = v.x * v.y;
	return result;
}


static inline f32 Componentwise_Add(v4f v)
{
	f32 result = v.x + v.y + v.z + v.w;
	return result;
}


static inline f32 Componentwise_Add(v2f v)
{
	f32 result = v.x + v.y;
	return result;
}


static inline i32 Round_To_Signed_Int32(f32 real)
{
	i32 result = (u32)(real + 0.5f);
	return result;
}


static inline v2i Round_To_Signed_Int32(v2f v)
{
	v += 0.5f;
	v2i result = v2f::Cast<i32>(v);	
	return result;
}


static inline f32 Lerp(f32 a, f32 b, f32 t)
{
	f32 result = a + t * (b - a);
	return result;
}


static inline v2f Lerp(v2f a, v2f b, f32 t)
{
	v2f result = { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
	return result;
}


static inline v3f Lerp(v3f a, v3f b, f32 t)
{
	v3f result = { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
	return result;
}


static inline v4f Lerp(v4f a, v4f b, f32 t)
{
	v4f result = { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t), Lerp(a.w, b.w, t) };
	return result;
}


static inline bool Is_Rect_Valid(Rect rect)
{
	bool result = rect.min.x < rect.max.x && rect.min.y < rect.max.y;
	return result;
}


static inline Rect Create_Rect_Center(v2f pos, v2f dim)
{
	dim *= 0.5f;
	Rect result = {pos - dim, pos + dim};
	Assert(Is_Rect_Valid(result));
	
	return result;
}


static inline Rect Create_Rect_Min_Max(v2f min, v2f max)
{
	Rect result = { min, max };
	Assert(Is_Rect_Valid(result));
	
	return result;
}


static inline Rect Create_Rect_Min_Dim(v2f min, v2f dim)
{
	Rect result = { min, min + dim };
	Assert(Is_Rect_Valid(result));
	
	return result;
}



static inline bool Is_Point_Inside_Rect(v2f p, Rect rect)
{
	bool result = p.x >= rect.min.x && p.y >= rect.min.y && p.x < rect.max.x && p.y < rect.max.y;	
	return result;
}
 
 
static inline f32 Clamp_To_Barycentric(f32 real)
{
	if(real != real)
		return 0;
	
	if(real > 1)
		return 1;
	
	if(real < 0)
		return 0;
	
	return real;
}