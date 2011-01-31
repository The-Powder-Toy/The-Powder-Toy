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
#if defined(WIN32) && !defined(__GNUC__)
extern _inline int isign(float i);
#else
extern inline int isign(float i);
#endif

#if defined(WIN32) && !defined(__GNUC__)
extern _inline unsigned clamp_flt(float f, float min, float max);
#else
extern inline unsigned clamp_flt(float f, float min, float max);
#endif

#if defined(WIN32) && !defined(__GNUC__)
extern _inline float restrict_flt(float f, float min, float max);
#else
extern inline float restrict_flt(float f, float min, float max);
#endif

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

void load_presets(void);

void save_string(FILE *f, char *str);

int sregexp(const char *str, char *pattern);

int load_string(FILE *f, char *str, int max);

void strcaturl(char *dst, char *src);

void strappend(char *dst, char *src);

void *file_load(char *fn, int *size);

int cpu_check(void);

#endif
