#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32) && !defined(__GNUC__)
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

static char hex[] = "0123456789ABCDEF";

//Signum function
int isign(float i);

unsigned clamp_flt(float f, float min, float max);

float restrict_flt(float f, float min, float max);

char *mystrdup(char *s);

struct strlist
{
	char *str;
	struct strlist *next;
};

void strlist_add(struct strlist **list, char *str);

int strlist_find(struct strlist **list, char *str);

void strlist_free(struct strlist **list);

void save_presets(int do_update);

void clean_text(char *text, int vwidth);

void load_presets(void);

void save_string(FILE *f, char *str);

int sregexp(const char *str, char *pattern);

int load_string(FILE *f, char *str, int max);

void strcaturl(char *dst, char *src);

void strappend(char *dst, char *src);

void *file_load(char *fn, int *size);

void clipboard_push_text(char * text);

char * clipboard_pull_text();

extern char *clipboard_text;

int register_extension();

int cpu_check(void);

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
