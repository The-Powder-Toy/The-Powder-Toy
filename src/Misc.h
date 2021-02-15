#ifndef UTILS_H
#define UTILS_H
#include "Config.h"
#include <cstdio>
#include <cstdlib>
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
	if (f<min)
		return min;
	if (f>max)
		return max;
	return f;
}

void save_presets(int do_update);

void load_presets(void);

void strcaturl(char *dst, const char *src);

void strappend(char *dst, const char *src);

void *file_load(char *fn, int *size);

extern char *clipboard_text;

void HSV_to_RGB(int h,int s,int v,int *r,int *g,int *b);

void RGB_to_HSV(int r,int g,int b,int *h,int *s,int *v);

void membwand(void * dest, void * src, size_t destsize, size_t srcsize);

// a b
// c d

struct matrix2d {
	float a,b,c,d;
};
typedef struct matrix2d matrix2d;

// column vector
struct vector2d {
	float x,y;
};
typedef struct vector2d vector2d;

matrix2d m2d_multiply_m2d(matrix2d m1, matrix2d m2);
vector2d m2d_multiply_v2d(matrix2d m, vector2d v);
matrix2d m2d_multiply_float(matrix2d m, float s);
vector2d v2d_multiply_float(vector2d v, float s);

vector2d v2d_add(vector2d v1, vector2d v2);
vector2d v2d_sub(vector2d v1, vector2d v2);

matrix2d m2d_new(float me0, float me1, float me2, float me3);
vector2d v2d_new(float x, float y);

extern vector2d v2d_zero;
extern matrix2d m2d_identity;

#endif
