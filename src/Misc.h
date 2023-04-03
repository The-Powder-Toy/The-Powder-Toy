#pragma once
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <vector>

//Linear interpolation
template <typename T> inline T LinearInterpolate(T val1, T val2, T lowerCoord, T upperCoord, T coord)
{
	if(lowerCoord == upperCoord) return val1;
	return (((val2 - val1) / (upperCoord - lowerCoord)) * (coord - lowerCoord)) + val1;
}


//Signum function
inline int isign(int i)
{
	if (i<0)
		return -1;
	if (i>0)
		return 1;
	return 0;
}

inline int isign(float i)
{
	if (i<0)
		return -1;
	if (i>0)
		return 1;
	return 0;
}

inline unsigned clamp_flt(float f, float min, float max)
{
	if (f<min)
		return 0;
	if (f>max)
		return 255;
	return (int)(255.0f*(f-min)/(max-min));
}

inline float restrict_flt(float f, float min, float max)
{
	// Fix crash in certain cases when f is nan
	if (!std::isfinite(f))
		return min;
	if (f < min)
		return min;
	if (f > max)
		return max;
	return f;
}

void HSV_to_RGB(int h,int s,int v,int *r,int *g,int *b);
void RGB_to_HSV(int r,int g,int b,int *h,int *s,int *v);
void membwand(void * dest, void * src, size_t destsize, size_t srcsize);

// a b
// c d
struct matrix2d {
	[[deprecated("Use Mat2")]]
	float a,b,c,d;
};
[[deprecated("Use Mat2")]]
typedef struct matrix2d matrix2d;

// column vector
struct vector2d {
	[[deprecated("Use Vec2")]]
	float x,y;
};
[[deprecated("Use Vec2")]]
typedef struct vector2d vector2d;

//matrix2d m2d_multiply_m2d(matrix2d m1, matrix2d m2);
[[deprecated("Use Mat2::operator*(Vec2)")]]
vector2d m2d_multiply_v2d(matrix2d m, vector2d v);
//matrix2d m2d_multiply_float(matrix2d m, float s);
[[deprecated("Use Vec2<float>::operator*(float)")]]
vector2d v2d_multiply_float(vector2d v, float s);

[[deprecated("Use Vec2::operator+")]]
vector2d v2d_add(vector2d v1, vector2d v2);
[[deprecated("Use Vec2::operator-")]]
vector2d v2d_sub(vector2d v1, vector2d v2);

[[deprecated("Use Mat2")]]
matrix2d m2d_new(float me0, float me1, float me2, float me3);
[[deprecated("Use Vec2")]]
vector2d v2d_new(float x, float y);

[[deprecated("Use Vec2::Zero")]]
extern vector2d v2d_zero;
[[deprecated("Use Mat2::Identity")]]
extern matrix2d m2d_identity;

class ByteString;

bool byteStringEqualsString(const ByteString &str, const char *data, size_t size);
template<size_t N>
// TODO: use std::literals::string_literals::operator""s if we get rid of ByteString
bool byteStringEqualsLiteral(const ByteString &str, const char (&lit)[N])
{
	return byteStringEqualsString(str, lit, N - 1U);
}
