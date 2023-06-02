#pragma once
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <vector>

template<class Signed>
inline std::pair<Signed, Signed> floorDiv(Signed a, Signed b)
{
	auto quo = a / b;
	auto rem = a % b;
	if (a < Signed(0) && rem)
	{
		quo -= Signed(1);
		rem += b;
	}
	return { quo, rem };
}

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

class ByteString;

bool byteStringEqualsString(const ByteString &str, const char *data, size_t size);
template<size_t N>
// TODO: use std::literals::string_literals::operator""s if we get rid of ByteString
bool byteStringEqualsLiteral(const ByteString &str, const char (&lit)[N])
{
	return byteStringEqualsString(str, lit, N - 1U);
}
