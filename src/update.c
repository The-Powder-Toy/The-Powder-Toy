/**
 * Powder Toy - Update Helper
 *
 * Copyright (c) 2008 - 2010 Stanislaw Skowronek.
 * Copyright (c) 2010 Simon Robertshaw
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
#include <stdlib.h>
#ifndef MACOSX
#include <malloc.h>
#endif
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#include <errno.h>
#include <sys/param.h>
#endif

#include <graphics.h>
#include <http.h>
#include <interface.h>
#include <misc.h>
#include <update.h>

int version_check = 1;
int version_check_http_counter = 0;
void *version_check_http = NULL;
int version_check_major = 0, version_check_minor = 0, version_check_isbeta = 0;
int old_ver_len = 0;
#ifdef BETA
static const char *old_ver_msg_beta = "A new beta is available - click here!";
#endif
static const char *old_ver_msg = "A new version is available - click here!";

char update_uri[] = "http://" SERVER "/Update.api?Action=Download&Architecture="
#if defined WIN32
                "Windows32"
#elif defined LIN32
                "Linux32"
#elif defined LIN64
                "Linux64"
#elif defined MACOSX
                "MacOSX"
#else
                "Unknown"
#endif
                "&InstructionSet="
#if defined X86_SSE3
                "SSE3"
#elif defined X86_SSE2
                "SSE2"
#elif defined X86_SSE
                "SSE"
#else
                "SSE"
#endif
                ;

char *exe_name(void)
{
#if defined WIN32
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
		name = realloc(name, max);
		memset(name, 0, max);
	}
#endif
	if (res <= 0)
	{
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

	if (!self)
		return 1;

#ifdef WIN32
	temp = malloc(strlen(self)+12);
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

	if ((int)ShellExecute(NULL, "open", self, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		DeleteFile(self);
		goto fail;
	}

	return 0;
#else
	temp = malloc(strlen(self)+8);
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
#ifdef WIN32
	char *temp, *self=exe_name(), *p;
	int timeout = 60, err;

	temp = malloc(strlen(self)+12);
	strcpy(temp, self);
	p = temp + strlen(temp) - 4;
	if (_stricmp(p, ".exe"))
		p += 4;
	strcpy(p, "_update.exe");

	while (!DeleteFile(temp))
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
		{
			// just as well, then
			free(temp);
			return 0;
		}
		Sleep(500);
		timeout--;
		if (timeout <= 0)
		{
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

void update_notify_ui_draw(pixel *vid_buf)
{
	if (version_check==2)
	{
		clearrect(vid_buf, XRES-21-old_ver_len, YRES-24, old_ver_len+9, 17);
#ifdef BETA
		if (version_check_isbeta)
		{
			drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg_beta, 255, 216, 32, 255);
		}
		else
		{
			drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
		}
#else
		drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
#endif
		drawrect(vid_buf, XRES-19-old_ver_len, YRES-22, old_ver_len+5, 13, 255, 216, 32, 255);
	}
}

void update_notify_ui_process(pixel *vid_buf, int mb, int mbq, int mx, int my)
{
	if (version_check==1 && !version_check_http)
		version_check_http = http_async_req_start(NULL, "http://" SERVER "/Update.api?Action=CheckVersion", NULL, 0, 0);
	if (version_check_http)
	{
		if (!version_check_http_counter && http_async_req_status(version_check_http))
		{
			int http_ret;
			char *ver_data;
			ver_data = http_async_req_stop(version_check_http, &http_ret, NULL);
			if (http_ret==200 && ver_data)
			{
#ifdef BETA
				if (sscanf(ver_data, "%d.%d.%d", &version_check_major, &version_check_minor, &version_check_isbeta)==3)
					if (version_check_major>SAVE_VERSION ||
						(version_check_major==SAVE_VERSION && version_check_minor>MINOR_VERSION) ||
						(version_check_major==SAVE_VERSION && version_check_isbeta == 0))
						version_check = 2;
#else
				if (sscanf(ver_data, "%d.%d", &version_check_major, &version_check_minor)==2)
					if (version_check_major>SAVE_VERSION || (version_check_major==SAVE_VERSION && version_check_minor>MINOR_VERSION))
						version_check = 2;
#endif
#ifdef BETA
				if (version_check_isbeta) old_ver_len = textwidth(old_ver_msg_beta);
				else old_ver_len = textwidth(old_ver_msg);
#else
				old_ver_len = textwidth(old_ver_msg);
#endif
			}
			if (ver_data) free(ver_data);
			version_check_http = NULL;
		}
		version_check_http_counter = (version_check_http_counter+1) & 15;
	}
	if (mb && !mbq && mx>=(XRES-19-old_ver_len) &&
		        mx<=(XRES-14) && my>=(YRES-22) && my<=(YRES-9) && version_check==2)
		{
			char *tmp = malloc(64);
#ifdef BETA
			if (version_check_isbeta)
			{
				sprintf(tmp, "Your version: %d (Beta %d), new version: %d (Beta %d).", SAVE_VERSION, MINOR_VERSION, version_check_major, version_check_minor);
			}
			else
			{
				sprintf(tmp, "Your version: %d (Beta %d), new version: %d.%d.", SAVE_VERSION, MINOR_VERSION, version_check_major, version_check_minor);
			}
#else
			sprintf(tmp, "Your version: %d.%d, new version: %d.%d.", SAVE_VERSION, MINOR_VERSION, version_check_major, version_check_minor);
#endif
			if (confirm_ui(vid_buf, "Do you want to update The Powder Toy?", tmp, "Update"))
			{
				int i;
				free(tmp);
				tmp = download_ui(vid_buf, update_uri, &i);
				if (tmp)
				{
					save_presets(1);
					if (update_start(tmp, i))
					{
						update_cleanup();
						save_presets(0);
						error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
					}
					else
						exit(0);
				}
			}
			else
				free(tmp);
		}
}

void update_finalize(pixel *vid_buf)
{
	if (!update_flag) return;
	info_box(vid_buf, "Finalizing update...");
	if (last_major>SAVE_VERSION || (last_major==SAVE_VERSION && last_minor>=MINOR_VERSION))
	{
		update_cleanup();
		error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
	}
	else
	{
		if (update_finish())
			error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
		else
			info_ui(vid_buf, "Update success", "You have successfully updated the Powder Toy!");
	}
	update_flag = 0;
}
