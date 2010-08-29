#include <stdlib.h>
#include <string.h>
#include "misc.h"

//Signum function
#ifdef WIN32
_inline int isign(float i)
#else
inline int isign(float i)
#endif
{
    if (i<0)
        return -1;
    if (i>0)
        return 1;
    return 0;
}

#ifdef WIN32
_inline unsigned clamp_flt(float f, float min, float max)
#else
inline unsigned clamp_flt(float f, float min, float max)
#endif
{
    if(f<min)
        return 0;
    if(f>max)
        return 255;
    return (int)(255.0f*(f-min)/(max-min));
}

#ifdef WIN32
_inline float restrict_flt(float f, float min, float max)
#else
inline float restrict_flt(float f, float min, float max)
#endif
{
    if(f<min)
        return min;
    if(f>max)
        return max;
    return f;
}

char *mystrdup(char *s)
{
    char *x;
    if(s)
    {
        x = malloc(strlen(s)+1);
        strcpy(x, s);
        return x;
    }
    return s;
}

void strlist_add(struct strlist **list, char *str)
{
    struct strlist *item = malloc(sizeof(struct strlist));
    item->str = mystrdup(str);
    item->next = *list;
    *list = item;
}

int strlist_find(struct strlist **list, char *str)
{
    struct strlist *item;
    for(item=*list; item; item=item->next)
        if(!strcmp(item->str, str))
            return 1;
    return 0;
}

void strlist_free(struct strlist **list)
{
    struct strlist *item;
    while(*list)
    {
        item = *list;
        *list = (*list)->next;
        free(item);
    }
}