#ifndef UTILS_H
#define UTILS_H
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

#if defined(WIN) && !defined(__GNUC__)
#define x86_cpuid(func,af,bf,cf,df) \
	do {\
	__asm mov	eax, func\
	__asm cpuid\
	__asm mov	af, eax\
	__asm mov	bf, ebx\
	__asm mov	cf, ecx\
	__asm mov	df, edx\
	} while(0)
#else
#define x86_cpuid(func,af,bf,cf,df) \
__asm__ __volatile ("cpuid":\
	"=a" (af), "=b" (bf), "=c" (cf), "=d" (df) : "a" (func));
#endif

//Linear interpolation
template <typename T> inline T LinearInterpolate(T val1, T val2, T lowerCoord, T upperCoord, T coord)
{
	if(lowerCoord == upperCoord) return val1;
	return (((val2 - val1) / (upperCoord - lowerCoord)) * (coord - lowerCoord)) + val1;
}

//Signum function
int isign(float i);

unsigned clamp_flt(float f, float min, float max);

float restrict_flt(float f, float min, float max);

char *mystrdup(const char *s);

struct strlist
{
	char *str;
	struct strlist *next;
};

void strlist_add(struct strlist **list, char *str);

int strlist_find(struct strlist **list, char *str);

void strlist_free(struct strlist **list);

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
