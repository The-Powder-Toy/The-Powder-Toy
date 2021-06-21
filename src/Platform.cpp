#include "Platform.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#ifdef WIN
#define NOMINMAX
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

#include "Misc.h"
#include "client/Client.h"

namespace Platform
{

ByteString ExecutableName()
{
	ByteString ret;
#if defined(WIN)
	using Char = wchar_t;
#elif defined(LIN)
	using Char = char;
#endif
#if defined(WIN)
	wchar_t *name = (wchar_t *)malloc(sizeof(wchar_t) * 64);
	DWORD max = 64, res;
	while ((res = GetModuleFileNameW(NULL, name, max)) >= max)
	{
#elif defined MACOSX
	char *fn = (char*)malloc(64),*name = (char*)malloc(PATH_MAX);
	uint32_t max = 64, res;
	if (_NSGetExecutablePath(fn, &max) != 0)
	{
		char *realloced_fn = (char*)realloc(fn, max);
		assert(realloced_fn != NULL);
		fn = realloced_fn;
		_NSGetExecutablePath(fn, &max);
	}
	if (realpath(fn, name) == NULL)
	{
		free(fn);
		free(name);
		return "";
	}
	res = 1;
#else
	char fn[64], *name = (char *)malloc(64);
	size_t max = 64, res;
	sprintf(fn, "/proc/self/exe");
	memset(name, 0, max);
	while ((res = readlink(fn, name, max)) >= max-1)
	{
#endif
#ifndef MACOSX
		max *= 2;
		Char* realloced_name = (Char *)realloc(name, sizeof(Char) * max);
		assert(realloced_name != NULL);
		name = realloced_name;
		memset(name, 0, sizeof(Char) * max);
	}
#endif
	if (res <= 0)
	{
		free(name);
		return "";
	}
#if defined(WIN)
	ret = WinNarrow(name);
#else
	ret = name;
#endif
	free(name);
	return ret;
}

void DoRestart()
{
	ByteString exename = ExecutableName();
	if (exename.length())
	{
#ifdef WIN
		int ret = int(INT_PTR(ShellExecuteW(NULL, NULL, WinWiden(exename).c_str(), NULL, NULL, SW_SHOWNORMAL)));
		if (ret <= 32)
		{
			fprintf(stderr, "cannot restart: ShellExecute(...) failed: code %i\n", ret);
		}
		else
		{
#if !defined(RENDERER) && !defined(FONTEDITOR)
			Client::Ref().Shutdown(); // very ugly hack; will fix soon(tm)
#endif
			exit(0);
		}
#elif defined(LIN) || defined(MACOSX)
		execl(exename.c_str(), "powder", NULL);
		int ret = errno;
		fprintf(stderr, "cannot restart: execl(...) failed: code %i\n", ret);
#endif
	}
	else
	{
		fprintf(stderr, "cannot restart: no executable name???\n");
	}
	exit(-1);
}

void OpenURI(ByteString uri)
{
#if defined(WIN)
	if (int(INT_PTR(ShellExecuteW(NULL, NULL, WinWiden(uri).c_str(), NULL, NULL, SW_SHOWNORMAL))) <= 32)
	{
		fprintf(stderr, "cannot open URI: ShellExecute(...) failed\n");
	}
#elif defined(MACOSX)
	if (system(("open \"" + uri + "\"").c_str()))
	{
		fprintf(stderr, "cannot open URI: system(...) failed\n");
	}
#elif defined(LIN)
	if (system(("xdg-open \"" + uri + "\"").c_str()))
	{
		fprintf(stderr, "cannot open URI: system(...) failed\n");
	}
#else
	fprintf(stderr, "cannot open URI: not implemented\n");
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


void LoadFileInResource(int name, int type, unsigned int& size, const char*& data)
{
#ifdef _MSC_VER
	HMODULE handle = ::GetModuleHandle(NULL);
	HRSRC rc = ::FindResource(handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));
	HGLOBAL rcData = ::LoadResource(handle, rc);
	size = ::SizeofResource(handle, rc);
	data = static_cast<const char*>(::LockResource(rcData));
#endif
}

#ifdef WIN
ByteString WinNarrow(const std::wstring &source)
{
	int buffer_size = WideCharToMultiByte(CP_UTF8, 0, source.c_str(), source.size(), nullptr, 0, NULL, NULL);
	if (!buffer_size)
	{
		return "";
	}
	std::string output(buffer_size, 0);
	if (!WideCharToMultiByte(CP_UTF8, 0, source.c_str(), source.size(), &output[0], buffer_size, NULL, NULL))
	{
		return "";
	}
	return output;
}

std::wstring WinWiden(const ByteString &source)
{
	int buffer_size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), nullptr, 0);
	if (!buffer_size)
	{
		return L"";
	}
	std::wstring output(buffer_size, 0);
	if (!MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), &output[0], buffer_size))
	{
		return L"";
	}
	return output;
}
#endif

}

#ifdef WIN
# undef main // thank you sdl
int main(int argc, char *argv[]);
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	int argc;
	wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::vector<ByteString> argv;
	for (auto i = 0; i < argc; ++i)
	{
		argv.push_back(Platform::WinNarrow(std::wstring(wargv[i])));
	}
	std::vector<char *> argp;
	for (auto &arg : argv)
	{
		argp.push_back(&arg[0]);
	}
	return main(argc, &argp[0]);
}
#endif
