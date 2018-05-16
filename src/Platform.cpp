#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#ifdef WIN
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#endif
#include "Platform.h"
#include "Misc.h"

namespace Platform
{
ByteString ExecutableName()
{
	ByteString ret;
#if defined(WIN)
	//char *name = (char *)malloc(64);
	DWORD max = 64, res;
	ByteString name(max, 0);
	while ((res = GetModuleFileName(NULL, &name[0], max)) >= max)
	{
#elif defined MACOSX
	char *fn = (char*)malloc(64),*name = (char*)malloc(PATH_MAX);
	int32_t max = 64, res;
	/* Possible fix for memory leak and removeing Char * Strings. */
	/*
	ByteString fn(max, 0);
	ByteString name(PATH_MAX, 0);
	if(_NSGetExecutablePath(fn.data(), &max) != 0)
	{
		fm.resize(max);
		_NSGetExecutablePath(fn.data(), &max);
	}
	*/
	if (_NSGetExecutablePath(fn, &max) != 0)
	{
		char *realloced_fn = (char*)realloc(fn, max);
		assert(realloced_fn != NULL);
		fn = realloced_fn;
		_NSGetExecutablePath(fn, &max);
	}
	if (realpath(fn, name) == NULL)
	{
		/*IN thefix above the two free calls would be removed*/
		free(fn);
		free(name);
		return "";
	}
	res = 1;
#else
	/*Changes here are estimations I Dont have a box to test if this code is correct
	  Something needs to be changed because the next preprocessor block uses the name 
	  from here and the WIN block*/
	char fn[64];//, *name = (char *)malloc(64);
	size_t max = 64, res;
	ByteString name(max, 0);
	sprintf(fn, "/proc/self/exe");
	//memset(name, 0, max);
	while ((res = readlink(fn, name.data(), max)) >= max - 1)
	{
#endif
#ifndef MACOSX
		max *= 2;
		//char* realloced_name = (char *)realloc(name, max);
		//assert(realloced_name != NULL);
		//name = realloced_name;
		//memset(name, 0, max);
		name.resize(max);
		std::fill_n(name.begin(), max, 0);
	}
#endif
	if (res <= 0)
	{
		//free(name);
		return "";
	}
	ret = name;
	//free(name);
	return ret;
}

void DoRestart()
{
	ByteString exename = ExecutableName();
	if (exename.length())
	{
#ifdef WIN
		ShellExecute(NULL, "open", exename.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(LIN) || defined(MACOSX)
		execl(exename.c_str(), "powder", NULL);
#endif
	}
	exit(-1);
}

void OpenURI(ByteString uri)
{
#if defined(WIN)
	ShellExecute(0, "OPEN", uri.c_str(), NULL, NULL, 0);
#elif defined(MACOSX)
	char *cmd = (char*)malloc(7+uri.length());
	strcpy(cmd, "open ");
	strappend(cmd, (char*)uri.c_str());
	system(cmd);
#elif defined(LIN)
	char *cmd = (char*)malloc(11+uri.length());
	strcpy(cmd, "xdg-open ");
	strappend(cmd, (char*)uri.c_str());
	system(cmd);
#else
	printf("Cannot open browser\n");
#endif
}

void Millisleep(long int t)
{
#ifdef WIN
	Sleep(t);
#else
	struct timespec s;
	s.tv_sec = t / 1000;
	s.tv_nsec = (t % 1000) * 10000000;
	nanosleep(&s, NULL);
#endif
}

long unsigned int GetTime()
{
#ifdef WIN
	return GetTickCount();
#elif defined(MACOSX)
	struct timeval s;
	gettimeofday(&s, NULL);
	return (unsigned int)(s.tv_sec * 1000 + s.tv_usec / 1000);
#else
	struct timespec s;
	clock_gettime(CLOCK_MONOTONIC, &s);
	return s.tv_sec * 1000 + s.tv_nsec / 1000000;
#endif
}

}
