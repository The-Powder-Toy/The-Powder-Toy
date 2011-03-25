#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include "misc.h"
#include "defines.h"
#include "interface.h"
#include "graphics.h"
#include "powder.h"

//Signum function
#if defined(WIN32) && !defined(__GNUC__)
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

#if defined(WIN32) && !defined(__GNUC__)
_inline unsigned clamp_flt(float f, float min, float max)
#else
inline unsigned clamp_flt(float f, float min, float max)
#endif
{
	if (f<min)
		return 0;
	if (f>max)
		return 255;
	return (int)(255.0f*(f-min)/(max-min));
}

#if defined(WIN32) && !defined(__GNUC__)
_inline float restrict_flt(float f, float min, float max)
#else
inline float restrict_flt(float f, float min, float max)
#endif
{
	if (f<min)
		return min;
	if (f>max)
		return max;
	return f;
}

char *mystrdup(char *s)
{
	char *x;
	if (s)
	{
		x = (char*)malloc(strlen(s)+1);
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
	for (item=*list; item; item=item->next)
		if (!strcmp(item->str, str))
			return 1;
	return 0;
}

void strlist_free(struct strlist **list)
{
	struct strlist *item;
	while (*list)
	{
		item = *list;
		*list = (*list)->next;
		free(item);
	}
}

void save_presets(int do_update)
{
	FILE *f=fopen("powder.def", "wb");
	unsigned char sig[4] = {0x50, 0x44, 0x65, 0x67};
	unsigned char tmp = sdl_scale;
	if (!f)
		return;
	fwrite(sig, 1, 4, f);
	save_string(f, svf_user);
	//save_string(f, svf_pass);
	save_string(f, svf_user_id);
	save_string(f, svf_session_id);
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

int sregexp(const char *str, char *pattern)
{
	int result;
	regex_t patternc;
	if(regcomp(&patternc, pattern,  0)!=0)
		return 1;
	result = regexec(&patternc, str, 0, NULL, 0);
	regfree(&patternc);
	return result;
}

void load_presets(void)
{
	FILE *f=fopen("powder.def", "rb");
	unsigned char sig[4], tmp;
	if (!f)
		return;
	fread(sig, 1, 4, f);
	if (sig[0]!=0x50 || sig[1]!=0x44 || sig[2]!=0x65)
	{
		if (sig[0]==0x4D && sig[1]==0x6F && sig[2]==0x46 && sig[3]==0x6F)
		{
			if (fseek(f, -3, SEEK_END))
			{
				remove("powder.def");
				return;
			}
			if (fread(sig, 1, 3, f) != 3)
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
	if(sig[3]==0x66){
		if (load_string(f, svf_user, 63))
			goto fail;
		if (load_string(f, svf_pass, 63))
			goto fail;
	} else {
		if (load_string(f, svf_user, 63))
			goto fail;
		if (load_string(f, svf_user_id, 63))
			goto fail;
		if (load_string(f, svf_session_id, 63))
			goto fail;
	}
	svf_login = !!svf_session_id[0];
	if (fread(&tmp, 1, 1, f) != 1)
		goto fail;
	sdl_scale = (tmp == 2) ? 2 : 1;
	if (fread(&tmp, 1, 1, f) != 1)
		goto fail;
	cmode = tmp%CM_COUNT;
	if (fread(&tmp, 1, 1, f) != 1)
		goto fail;
	svf_admin = tmp;
	if (fread(&tmp, 1, 1, f) != 1)
		goto fail;
	svf_mod = tmp;
	if (load_string(f, http_proxy_string, 255))
		goto fail;
	if (fread(sig, 1, 3, f) != 3)
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
	if (li > max)
	{
		str[0] = 0;
		return 1;
	}
	fread(str, li, 1, f);
	str[li] = 0;
	return 0;
}

void strcaturl(char *dst, char *src)
{
	char *d;
	unsigned char *s;

	for (d=dst; *d; d++) ;

	for (s=(unsigned char *)src; *s; s++)
	{
		if ((*s>='0' && *s<='9') ||
		        (*s>='a' && *s<='z') ||
		        (*s>='A' && *s<='Z'))
			*(d++) = *s;
		else
		{
			*(d++) = '%';
			*(d++) = hex[*s>>4];
			*(d++) = hex[*s&15];
		}
	}
	*d = 0;
}

void strappend(char *dst, char *src)
{
	char *d;
	unsigned char *s;

	for (d=dst; *d; d++) ;

	for (s=(unsigned char *)src; *s; s++)
	{
		*(d++) = *s;
	}
	*d = 0;
}

void *file_load(char *fn, int *size)
{
	FILE *f = fopen(fn, "rb");
	void *s;

	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	s = malloc(*size);
	if (!s)
	{
		fclose(f);
		return NULL;
	}
	fread(s, *size, 1, f);
	fclose(f);
	return s;
}

int cpu_check(void)
{
#ifdef MACOSX
	return 0;
#else
#ifdef X86
	unsigned af,bf,cf,df;
	x86_cpuid(0, af, bf, cf, df);
	if (bf==0x68747541 && cf==0x444D4163 && df==0x69746E65)
		amd = 1;
	x86_cpuid(1, af, bf, cf, df);
#ifdef X86_SSE
	if (!(df&(1<<25)))
		return 1;
#endif
#ifdef X86_SSE2
	if (!(df&(1<<26)))
		return 1;
#endif
#ifdef X86_SSE3
	if (!(cf&1))
		return 1;
#endif
#endif
#endif
	return 0;
}

matrix2d m2d_multiply_m2d(matrix2d m1, matrix2d m2)
{
	matrix2d result = {
		m1.a*m2.a+m1.b*m2.c, m1.a*m2.b+m1.b*m2.d,
		m1.c*m2.a+m1.d*m2.c, m1.c*m2.b+m1.d*m2.d
	};
	return result;
}
vector2d m2d_multiply_v2d(matrix2d m, vector2d v)
{
	vector2d result = {
		m.a*v.x+m.b*v.y,
		m.c*v.x+m.d*v.y
	};
	return result;
}
matrix2d m2d_multiply_float(matrix2d m, float s)
{
	matrix2d result = {
		m.a*s, m.b*s,
		m.c*s, m.d*s,
	};
	return result;
}

vector2d v2d_multiply_float(vector2d v, float s)
{
	vector2d result = {
		v.x*s,
		v.y*s
	};
	return result;
}

vector2d v2d_add(vector2d v1, vector2d v2)
{
	vector2d result = {
		v1.x+v2.x,
		v1.y+v2.y
	};
	return result;
}
vector2d v2d_sub(vector2d v1, vector2d v2)
{
	vector2d result = {
		v1.x-v2.x,
		v1.y-v2.y
	};
	return result;
}

matrix2d m2d_new(float me0, float me1, float me2, float me3)
{
	matrix2d result = {me0,me1,me2,me3};
	return result;
}
vector2d v2d_new(float x, float y)
{
	vector2d result = {x, y};
	return result;
}

void clipboard_push_text(char * text)
{
	printf("Not implemented: put text on clipboard \"%s\"\n", text);
}

char * clipboard_pull_text()
{
	printf("Not implemented: get text from clipboard\n");
	return "";
}

int register_extension()
{
#if defined WIN32
	
	LONG rresult;
	HKEY newkey;
	char *currentfilename;
	char *iconname;
	char *opencommand;
	currentfilename = exe_name();
	iconname = malloc(strlen(currentfilename)+3);
	opencommand = malloc(strlen(currentfilename)+13);
	sprintf(iconname, "%s,1", currentfilename);
	sprintf(opencommand, "\"%s\" open:\"%%1\"", currentfilename);
	
	//Create extension entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\.cps", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if(rresult != ERROR_SUCCESS){
		return 0;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"PowderToySave", strlen("PowderToySave")+1);
	if(rresult != ERROR_SUCCESS){
		RegCloseKey(newkey);
		return 0;
	}
	RegCloseKey(newkey);
	
	//Create program entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if(rresult != ERROR_SUCCESS){
		return 0;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"Powder Toy Save", strlen("Powder Toy Save")+1);
	if(rresult != ERROR_SUCCESS){
		RegCloseKey(newkey);
		return 0;
	}
	RegCloseKey(newkey);
	
	//Set DefaultIcon
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave\\DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if(rresult != ERROR_SUCCESS){
		return 0;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)iconname, strlen(iconname)+1);
	if(rresult != ERROR_SUCCESS){
		RegCloseKey(newkey);
		return 0;
	}
	RegCloseKey(newkey);
	
	//Set Launch command
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave\\shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if(rresult != ERROR_SUCCESS){
		return 0;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)opencommand, strlen(opencommand)+1);
	if(rresult != ERROR_SUCCESS){
		RegCloseKey(newkey);
		return 0;
	}
	RegCloseKey(newkey);
	
	return 1;
#elif defined LIN32
	return 0;
#elif defined LIN64
	return 0;
#elif defined MACOSX
	return 0;
#endif
}

vector2d v2d_zero = {0,0};
matrix2d m2d_identity = {1,0,0,1};
