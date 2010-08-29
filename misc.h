#ifndef UTILS_H
#define UTILS_H

//Signum function
#ifdef WIN32
extern _inline int isign(float i);
#else
extern inline int isign(float i);
#endif

#ifdef WIN32
extern _inline unsigned clamp_flt(float f, float min, float max);
#else
extern inline unsigned clamp_flt(float f, float min, float max);
#endif

#ifdef WIN32
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

#endif