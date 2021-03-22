/**
 * Powder Toy - Update helper
 *
 * Powder Toy is the legal property of Stanislaw Skowronek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <stdio.h>
#ifndef MACOSX
#include <malloc.h>
#endif
#include <string.h>

#ifdef WIN32
#include <windows.h>
int __cdecl _stricmp(const char*, const char*);
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#include <errno.h>
#include <sys/param.h>
#endif

#include "update.h"

static char *exe_name(void)
{
#if defined WIN32
    char *name=malloc(64), max=64, res;
    while((res = GetModuleFileName(NULL, name, max)) >= max) {
#elif defined MACOSX
    char *fn=malloc(64),*name=malloc(PATH_MAX), max=64, res;
    if(_NSGetExecutablePath(fn, &max) != 0) {
	fn = realloc(fn, max);
	_NSGetExecutablePath(fn, &max);
    }
    if(realpath(fn, name) == NULL) {
	free(fn);
	free(name);
	return NULL;
    }
    res = 1;
#else
    char fn[64], *name=malloc(64), max=64, res;
    sprintf(fn, "/proc/self/exe");
    memset(name, 0, max);
    while((res = readlink(fn, name, max)) >= max-1) {
#endif
#ifndef MACOSX
	max *= 2;
	name = realloc(name, max);
	memset(name, 0, max);
    }
#endif
    if(res <= 0) {
	free(name);
	return NULL;
    }
    return name;
}

int update_start(char *data, int len)
{
    char *self=exe_name(), *temp;
#ifdef WIN32
    char *p;
#endif
    FILE *f;
    int res = 1;

    if(!self)
	return 1;

#ifdef WIN32
    temp = malloc(strlen(self)+12);
    strcpy(temp, self);
    p = temp + strlen(temp) - 4;
    if(_stricmp(p, ".exe"))
	p += 4;
    strcpy(p, "_update.exe");

    if(!MoveFile(self, temp))
	goto fail;

    f = fopen(self, "wb");
    if(!f)
	goto fail;
    if(fwrite(data, 1, len, f) != len) {
	fclose(f);
	DeleteFile(self);
	goto fail;
    }
    fclose(f);

    if((int)ShellExecute(NULL, "open", self, NULL, NULL, SW_SHOWNORMAL) <= 32) {
	DeleteFile(self);
	goto fail;
    }
    
    return 0;
#else
    temp = malloc(strlen(self)+8);
    strcpy(temp, self);
    strcat(temp, "-update");

    f = fopen(temp, "w");
    if(!f)
	goto fail;
    if(fwrite(data, 1, len, f) != len) {
	fclose(f);
	unlink(temp);
	goto fail;
    }
    fclose(f);

    if(chmod(temp, 0755)) {
	unlink(temp);
	goto fail;
    }

    if(rename(temp, self)) {
	unlink(temp);
	goto fail;
    }

    execl(self, "powder-update", NULL);
#endif

fail:
    free(temp);
    free(self);
    return res;
}

int update_finish(void)
{
#ifdef WIN32
    char *temp, *self=exe_name(), *p;
    int timeout = 60, err;

    temp = malloc(strlen(self)+12);
    strcpy(temp, self);
    p = temp + strlen(temp) - 4;
    if(_stricmp(p, ".exe"))
	p += 4;
    strcpy(p, "_update.exe");

    while(!DeleteFile(temp)) {
	err = GetLastError();
	if(err == ERROR_FILE_NOT_FOUND) {
	    // just as well, then
	    free(temp);
	    return 0;
	}
	Sleep(500);
	timeout--;
	if(timeout <= 0) {
	    free(temp);
	    return 1;
	}
    }
    free(temp);
#endif
    return 0;
}

void update_cleanup(void)
{
#ifdef WIN32
    update_finish();
#endif
}
