#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "defines.h"
#include "interface.h"
#include "graphics.h"

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

void save_presets(int do_update)
{
    FILE *f=fopen("powder.def", "wb");
    unsigned char sig[4] = {0x50, 0x44, 0x65, 0x66};
    unsigned char tmp = sdl_scale;
    if(!f)
        return;
    fwrite(sig, 1, 4, f);
    save_string(f, svf_user);
    save_string(f, svf_pass);
    fwrite(&tmp, 1, 1, f);
    tmp = cmode;
    fwrite(&tmp, 1, 1, f);
    tmp = svf_admin;
    fwrite(&tmp, 1, 1, f);
    tmp = svf_mod;
    fwrite(&tmp, 1, 1, f);
    save_string(f, http_proxy_string);
    tmp = SAVE_VERSION;
    fwrite(&tmp, 1, 1, f);
    tmp = MINOR_VERSION;
    fwrite(&tmp, 1, 1, f);
    tmp = do_update;
    fwrite(&tmp, 1, 1, f);
    fclose(f);
}

void load_presets(void)
{
    FILE *f=fopen("powder.def", "rb");
    unsigned char sig[4], tmp;
    if(!f)
        return;
    fread(sig, 1, 4, f);
    if(sig[0]!=0x50 || sig[1]!=0x44 || sig[2]!=0x65 || sig[3]!=0x66)
    {
        if(sig[0]==0x4D && sig[1]==0x6F && sig[2]==0x46 && sig[3]==0x6F)
        {
            if(fseek(f, -3, SEEK_END))
            {
                remove("powder.def");
                return;
            }
            if(fread(sig, 1, 3, f) != 3)
            {
                remove("powder.def");
                goto fail;
            }
            last_major = sig[0];
            last_minor = sig[1];
            update_flag = sig[2];
        }
        fclose(f);
        remove("powder.def");
        return;
    }
    if(load_string(f, svf_user, 63))
        goto fail;
    if(load_string(f, svf_pass, 63))
        goto fail;
    svf_login = !!svf_user[0];
    if(fread(&tmp, 1, 1, f) != 1)
        goto fail;
    sdl_scale = (tmp == 2) ? 2 : 1;
    if(fread(&tmp, 1, 1, f) != 1)
        goto fail;
    cmode = tmp%7;
    if(fread(&tmp, 1, 1, f) != 1)
        goto fail;
    svf_admin = tmp;
    if(fread(&tmp, 1, 1, f) != 1)
        goto fail;
    svf_mod = tmp;
    if(load_string(f, http_proxy_string, 255))
        goto fail;
    if(fread(sig, 1, 3, f) != 3)
        goto fail;
    last_major = sig[0];
    last_minor = sig[1];
    update_flag = sig[2];
fail:
    fclose(f);
}

void save_string(FILE *f, char *str)
{
    int li = strlen(str);
    unsigned char lb[2];
    lb[0] = li;
    lb[1] = li >> 8;
    fwrite(lb, 2, 1, f);
    fwrite(str, li, 1, f);
}

int load_string(FILE *f, char *str, int max)
{
    int li;
    unsigned char lb[2];
    fread(lb, 2, 1, f);
    li = lb[0] | (lb[1] << 8);
    if(li > max)
    {
        str[0] = 0;
        return 1;
    }
    fread(str, li, 1, f);
    str[li] = 0;
    return 0;
}