#include <cstdio>
#include <cstdlib>
#ifndef WIN
#include <sys/param.h>
#endif
#if !defined(MACOSX) && !defined(BSD)
#include <malloc.h>
#endif
#include <cstring>
#include <cstdint>

#ifdef WIN
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#include <errno.h>
#endif

#include "Update.h"
#include "Platform.h"

// returns 1 on failure, 0 on success
int update_start(char *data, unsigned int len)
{
	ByteString exeName = Platform::ExecutableName(), updName;
	FILE *f;

	if (!exeName.length())
		return 1;

#ifdef WIN
	updName = exeName;
	ByteString extension = exeName.substr(exeName.length() - 4);
	if (extension == ".exe")
		updName = exeName.substr(0, exeName.length() - 4);
	updName = updName + "_upd.exe";

	if (!MoveFile(exeName.c_str(), updName.c_str()))
		return 1;

	f = fopen(exeName.c_str(), "wb");
	if (!f)
		return 1;
	if (fwrite(data, 1, len, f) != len)
	{
		fclose(f);
		DeleteFile(exeName.c_str());
		return 1;
	}
	fclose(f);

	if ((uintptr_t)ShellExecute(NULL, "open", exeName.c_str(), NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		DeleteFile(exeName.c_str());
		return 1;
	}

	return 0;
#else
	updName = exeName + "-update";

	f = fopen(updName.c_str(), "w");
	if (!f)
		return 1;
	if (fwrite(data, 1, len, f) != len)
	{
		fclose(f);
		unlink(updName.c_str());
		return 1;
	}
	fclose(f);

	if (chmod(updName.c_str(), 0755))
	{
		unlink(updName.c_str());
		return 1;
	}

	if (rename(updName.c_str(), exeName.c_str()))
	{
		unlink(updName.c_str());
		return 1;
	}

	execl(exeName.c_str(), "powder-update", NULL);
	return 0;
#endif
}

// returns 1 on failure, 0 on success
int update_finish()
{
#ifdef WIN
	ByteString exeName = Platform::ExecutableName(), updName;
	int timeout = 5, err;

#ifdef DEBUG
	printf("Update: Current EXE name: %s\n", exeName.c_str());
#endif

	updName = exeName;
	ByteString extension = exeName.substr(exeName.length() - 4);
	if (extension == ".exe")
		updName = exeName.substr(0, exeName.length() - 4);
	updName = updName + "_upd.exe";

#ifdef DEBUG
	printf("Update: Temp EXE name: %s\n", updName.c_str());
#endif

	while (!DeleteFile(updName.c_str()))
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
		{
#ifdef DEBUG
			printf("Update: Temp file not deleted\n");
#endif
			// Old versions of powder toy name their update files with _update.exe, delete that upgrade file here
			updName = exeName;
			ByteString extension = exeName.substr(exeName.length() - 4);
			if (extension == ".exe")
				updName = exeName.substr(0, exeName.length() - 4);
			updName = updName + "_update.exe";
			DeleteFile(updName.c_str());
			return 0;
		}
		Sleep(500);
		timeout--;
		if (timeout <= 0)
		{
#ifdef DEBUG
			printf("Update: Delete timeout\n");
#endif
			return 1;
		}
	}
#endif
	return 0;
}

void update_cleanup()
{
#ifdef WIN
	update_finish();
#endif
}
