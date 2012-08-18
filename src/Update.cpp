#include <stdio.h>
#include <stdlib.h>
#ifndef WIN
#include <sys/param.h>
#endif
#if !defined(MACOSX) && !defined(BSD)
#include <malloc.h>
#endif
#include <string.h>

#ifdef WIN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#include <errno.h>
#endif

#include <Update.h>
#include <Misc.h>

/*char *exe_name(void)
{
#if defined(WIN)
	char *name= (char *)malloc(64);
	DWORD max=64, res;
	while ((res = GetModuleFileName(NULL, name, max)) >= max)
	{
#elif defined MACOSX
	char *fn=malloc(64),*name=malloc(PATH_MAX);
	uint32_t max=64, res;
	if (_NSGetExecutablePath(fn, &max) != 0)
	{
		fn = realloc(fn, max);
		_NSGetExecutablePath(fn, &max);
	}
	if (realpath(fn, name) == NULL)
	{
		free(fn);
		free(name);
		return NULL;
	}
	res = 1;
#else
	char fn[64], *name=malloc(64);
	size_t max=64, res;
	sprintf(fn, "/proc/self/exe");
	memset(name, 0, max);
	while ((res = readlink(fn, name, max)) >= max-1)
	{
#endif
#ifndef MACOSX
		max *= 2;
		name = (char*)realloc(name, max);
		memset(name, 0, max);
	}
#endif
	if (res <= 0)
	{
		free(name);
		return NULL;
	}
	return name;
}*/

int update_start(char *data, int len)
{
	char *self=exe_name(), *temp;
#ifdef WIN
	char *p;
#endif
	FILE *f;
	int res = 1;

	if (!self)
		return 1;

#ifdef WIN
	temp = (char*)malloc(strlen(self)+12);
	strcpy(temp, self);
	p = temp + strlen(temp) - 4;
	if (_stricmp(p, ".exe"))
		p += 4;
	strcpy(p, "_update.exe");

	if (!MoveFile(self, temp))
		goto fail;

	f = fopen(self, "wb");
	if (!f)
		goto fail;
	if (fwrite(data, 1, len, f) != len)
	{
		fclose(f);
		DeleteFile(self);
		goto fail;
	}
	fclose(f);

	if ((uintptr_t)ShellExecute(NULL, "open", self, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		DeleteFile(self);
		goto fail;
	}

	return 0;
#else
	temp = (char*)malloc(strlen(self)+8);
	strcpy(temp, self);
	strcat(temp, "-update");

	f = fopen(temp, "w");
	if (!f)
		goto fail;
	if (fwrite(data, 1, len, f) != len)
	{
		fclose(f);
		unlink(temp);
		goto fail;
	}
	fclose(f);

	if (chmod(temp, 0755))
	{
		unlink(temp);
		goto fail;
	}

	if (rename(temp, self))
	{
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
#ifdef WIN
	char *temp, *self=exe_name(), *p;
	int timeout = 60, err;

#ifdef DEBUG
	printf("Update: Current EXE name: %s\n", self);
#endif

	temp = (char*)malloc(strlen(self)+12);
	strcpy(temp, self);
	p = temp + strlen(temp) - 4;
	if (_stricmp(p, ".exe"))
		p += 4;
	strcpy(p, "_update.exe");

#ifdef DEBUG
	printf("Update: Temp EXE name: %s\n", temp);
#endif

	while (!DeleteFile(temp))
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
		{
#ifdef DEBUG
	printf("Update: Temp file deleted\n");
#endif
			free(temp);
			return 0;
		}
		Sleep(500);
		timeout--;
		if (timeout <= 0)
		{
#ifdef DEBUG
			printf("Update: Delete timeout\n");
#endif
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
#ifdef WIN
	update_finish();
#endif
}
