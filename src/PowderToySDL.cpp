#ifdef USE_SDL

#include <map>
#include <string>
#include <ctime>
#ifdef WIN
#define _WIN32_WINNT 0x0501	//Necessary for some macros and functions, tells windows.h to include functions only available in Windows XP or later
#include <direct.h>
#endif
#include "SDLCompat.h"

#ifdef X86_SSE
#include <xmmintrin.h>
#endif
#ifdef X86_SSE3
#include <pmmintrin.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include "Config.h"
#include "graphics/Graphics.h"
#if defined(LIN)
#include "icon.h"
#endif
#include <signal.h>
#include <stdexcept>

#ifndef WIN
#include <unistd.h>
#endif
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
extern "C" {
	char * readClipboard();
	void writeClipboard(const char * clipboardData);	
}
#endif

#include "Format.h"

#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "simulation/SaveRenderer.h"
#include "client/Client.h"
#include "Misc.h"

#include "gui/game/GameController.h"
#include "gui/game/GameView.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"

#include "client/HTTP.h"

using namespace std;

#define INCLUDE_SYSWM
#include "SDLCompat.h"
#if defined(USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
SDL_SysWMinfo sdl_wminfo;
Atom XA_CLIPBOARD, XA_TARGETS, XA_UTF8_STRING;
#endif

std::string clipboardText = "";

int desktopWidth = 1280, desktopHeight = 1024;

SDL_Surface * sdl_scrn;
int scale = 1;
bool fullscreen = false;

void ClipboardPush(std::string text)
{
	clipboardText = text;
#ifdef MACOSX
	writeClipboard(text.c_str());
#elif defined(WIN)
	if (OpenClipboard(NULL))
	{
		HGLOBAL cbuffer;
		char * glbuffer;

		EmptyClipboard();

		cbuffer = GlobalAlloc(GMEM_DDESHARE, text.size() + 1);
		glbuffer = (char*)GlobalLock(cbuffer);

		strcpy(glbuffer, text.c_str());

		GlobalUnlock(cbuffer);
		SetClipboardData(CF_TEXT, cbuffer);
		CloseClipboard();
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	sdl_wminfo.info.x11.lock_func();
	XSetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
	XFlush(sdl_wminfo.info.x11.display);
	sdl_wminfo.info.x11.unlock_func();
#else
	printf("Not implemented: put text on clipboard \"%s\"\n", text.c_str());
#endif
}

void EventProcess(SDL_Event event);

std::string ClipboardPull()
{
#ifdef MACOSX
	const char *text = readClipboard();
	return text ? std::string(text) : "";
#elif defined(WIN)
	if (OpenClipboard(NULL))
	{
		HANDLE cbuffer;
		char * glbuffer;

		cbuffer = GetClipboardData(CF_TEXT);
		glbuffer = (char*)GlobalLock(cbuffer);
		GlobalUnlock(cbuffer);
		CloseClipboard();
		return glbuffer ? std::string(glbuffer) : "";
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	std::string text = "";
	Window selectionOwner;
	sdl_wminfo.info.x11.lock_func();
	selectionOwner = XGetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD);
	if (selectionOwner != None)
	{
		unsigned char *data = NULL;
		Atom type;
		int format, result;
		unsigned long len, bytesLeft;
		XConvertSelection(sdl_wminfo.info.x11.display, XA_CLIPBOARD, XA_UTF8_STRING, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
		XFlush(sdl_wminfo.info.x11.display);
		sdl_wminfo.info.x11.unlock_func();
		while (1)
		{
			SDL_Event event;
			SDL_WaitEvent(&event);
			if (event.type == SDL_SYSWMEVENT)
			{
				XEvent xevent = event.syswm.msg->event.xevent;
				if (xevent.type == SelectionNotify && xevent.xselection.requestor == sdl_wminfo.info.x11.window)
					break;
				else
					EventProcess(event);
			}
			else
				EventProcess(event);
		}
		sdl_wminfo.info.x11.lock_func();
		XGetWindowProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD, 0, 0, 0, AnyPropertyType, &type, &format, &len, &bytesLeft, &data);
		if (data)
		{
			XFree(data);
			data = NULL;
		}
		if (bytesLeft)
		{
			result = XGetWindowProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD, 0, bytesLeft, 0, AnyPropertyType, &type, &format, &len, &bytesLeft, &data);
			if (result == Success)
			{
				text = data ? (const char*)data : "";
				XFree(data);
			}
			else
			{
				printf("Failed to pull from clipboard\n");
				return "?";
			}
		}
		else
			return "";
		XDeleteProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD);
	}
	sdl_wminfo.info.x11.unlock_func();
	return text;
#else
	printf("Not implemented: get text from clipboard\n");
#endif
	return clipboardText;
}

int mousex = 0, mousey = 0;
#ifdef OGLI
void blit()
{
	SDL_GL_SwapBuffers();
}
#else
void DrawPixel(pixel * vid, pixel color, int x, int y)
{
	if (x >= 0 && x < WINDOWW && y >= 0 && y < WINDOWH)
		vid[x+y*WINDOWW] = color;
}
// draws a custom cursor, used to make 3D mode work properly (normal cursor ruins the effect)
void DrawCursor(pixel * vid)
{
	for (int j = 0; j <= 9; j++)
	{
		for (int i = 0; i <= j; i++)
		{
			if (i == 0 || i == j)
				DrawPixel(vid, 0xFFFFFFFF, mousex+i, mousey+j);
			else
				DrawPixel(vid, 0xFF000000, mousex+i, mousey+j);
		}
	}
	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+10);
	for (int i = 0; i < 5; i++)
	{
		DrawPixel(vid, 0xFF000000, mousex+1+i, mousey+10);
		DrawPixel(vid, 0xFFFFFFFF, mousex+6+i, mousey+10);
	}
	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+1, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+2, mousey+11);
	DrawPixel(vid, 0xFFFFFFFF, mousex+3, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+4, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+5, mousey+11);
	DrawPixel(vid, 0xFFFFFFFF, mousex+6, mousey+11);

	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+12);
	DrawPixel(vid, 0xFF000000, mousex+1, mousey+12);
	DrawPixel(vid, 0xFFFFFFFF, mousex+2, mousey+12);
	DrawPixel(vid, 0xFFFFFFFF, mousex+4, mousey+12);
	DrawPixel(vid, 0xFF000000, mousex+5, mousey+12);
	DrawPixel(vid, 0xFF000000, mousex+6, mousey+12);
	DrawPixel(vid, 0xFFFFFFFF, mousex+7, mousey+12);

	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+13);
	DrawPixel(vid, 0xFFFFFFFF, mousex+1, mousey+13);
	DrawPixel(vid, 0xFFFFFFFF, mousex+4, mousey+13);
	DrawPixel(vid, 0xFF000000, mousex+5, mousey+13);
	DrawPixel(vid, 0xFF000000, mousex+6, mousey+13);
	DrawPixel(vid, 0xFFFFFFFF, mousex+7, mousey+13);

	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+14);
	for (int i = 0; i < 2; i++)
	{
		DrawPixel(vid, 0xFFFFFFFF, mousex+5, mousey+14+i);
		DrawPixel(vid, 0xFF000000, mousex+6, mousey+14+i);
		DrawPixel(vid, 0xFF000000, mousex+7, mousey+14+i);
		DrawPixel(vid, 0xFFFFFFFF, mousex+8, mousey+14+i);

		DrawPixel(vid, 0xFFFFFFFF, mousex+6, mousey+16+i);
		DrawPixel(vid, 0xFF000000, mousex+7, mousey+16+i);
		DrawPixel(vid, 0xFF000000, mousex+8, mousey+16+i);
		DrawPixel(vid, 0xFFFFFFFF, mousex+9, mousey+16+i);
	}

	DrawPixel(vid, 0xFFFFFFFF, mousex+7, mousey+18);
	DrawPixel(vid, 0xFFFFFFFF, mousex+8, mousey+18);
}
void blit(pixel * vid)
{
	if (sdl_scrn)
	{
		int depth3d = ui::Engine::Ref().Get3dDepth();
		if (depth3d)
			DrawCursor(vid);
		pixel * src = vid;
		int j, x = 0, y = 0, w = WINDOWW, h = WINDOWH, pitch = WINDOWW;
		pixel *dst;
		if (SDL_MUSTLOCK(sdl_scrn))
			if (SDL_LockSurface(sdl_scrn)<0)
				return;
		dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
		if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
		{
			//pixel format conversion, used for strange formats (OS X specifically)
			int i;
			unsigned int red, green, blue;
			pixel px, lastpx, nextpx;
			SDL_PixelFormat *fmt = sdl_scrn->format;
			if(depth3d)
			{
				for (j=0; j<h; j++)
				{
					for (i=0; i<w; i++)
					{
						lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
						nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
						int redshift = PIXB(lastpx) + PIXG(lastpx);
						if (redshift > 255)
							redshift = 255;
						int blueshift = PIXR(nextpx) + PIXG(nextpx);
						if (blueshift > 255)
							blueshift = 255;
						red = ((int)(PIXR(lastpx)*.69f+redshift*.3f)>>fmt->Rloss)<<fmt->Rshift;
						green = ((int)(PIXG(nextpx)*.3f)>>fmt->Gloss)<<fmt->Gshift;
						blue = ((int)(PIXB(nextpx)*.69f+blueshift*.3f)>>fmt->Bloss)<<fmt->Bshift;
						dst[i] = red|green|blue;
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
					src+=pitch;
				}
			}
			else
			{
				for (j=0; j<h; j++)
				{
					for (i=0; i<w; i++)
					{
						px = src[i];
						red = (PIXR(px)>>fmt->Rloss)<<fmt->Rshift;
						green = (PIXG(px)>>fmt->Gloss)<<fmt->Gshift;
						blue = (PIXB(px)>>fmt->Bloss)<<fmt->Bshift;
						dst[i] = red|green|blue;
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
					src+=pitch;
				}
			}
		}
		else
		{
			int i;
			if(depth3d)
			{
				pixel lastpx, nextpx;
				for (j=0; j<h; j++)
				{
					for (i=0; i<w; i++)
					{
						lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
						nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
						int redshift = PIXB(lastpx) + PIXG(lastpx);
						if (redshift > 255)
							redshift = 255;
						int blueshift = PIXR(nextpx) + PIXG(nextpx);
						if (blueshift > 255)
							blueshift = 255;
						dst[i] = PIXRGB((int)(PIXR(lastpx)*.69f+redshift*.3f), (int)(PIXG(nextpx)*.3f), (int)(PIXB(nextpx)*.69f+blueshift*.3f));
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
					src+=pitch;
				}
			}
			else
			{
				for (j=0; j<h; j++)
				{
					memcpy(dst, src, w*PIXELSIZE);
					dst+=sdl_scrn->pitch/PIXELSIZE;
					src+=pitch;
				}
			}
		}
		if (SDL_MUSTLOCK(sdl_scrn))
			SDL_UnlockSurface(sdl_scrn);
		SDL_UpdateRect(sdl_scrn,0,0,0,0);
	}
}
void blit2(pixel * vid, int currentScale)
{
	if (sdl_scrn)
	{
		int depth3d = ui::Engine::Ref().Get3dDepth();
		if (depth3d)
			DrawCursor(vid);
		pixel * src = vid;
		int j, x = 0, y = 0, w = WINDOWW, h = WINDOWH, pitch = WINDOWW;
		pixel *dst;
		pixel px, lastpx, nextpx;
		int i,k;
		if (SDL_MUSTLOCK(sdl_scrn))
			if (SDL_LockSurface(sdl_scrn)<0)
				return;
		dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
		if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
		{
			//pixel format conversion
			SDL_PixelFormat *fmt = sdl_scrn->format;
			int red, green, blue;
			for (j=0; j<h; j++)
			{
				for (k=0; k<currentScale; k++)
				{
					for (i=0; i<w; i++)
					{
						if (depth3d)
						{
							lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
							nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
							int redshift = PIXB(lastpx) + PIXG(lastpx);
							if (redshift > 255)
								redshift = 255;
							int blueshift = PIXR(nextpx) + PIXG(nextpx);
							if (blueshift > 255)
								blueshift = 255;
							red = ((int)(PIXR(lastpx)*.69f+redshift*.3f)>>fmt->Rloss)<<fmt->Rshift;
							green = ((int)(PIXG(nextpx)*.3f)>>fmt->Gloss)<<fmt->Gshift;
							blue = ((int)(PIXB(nextpx)*.69f+blueshift*.3f)>>fmt->Bloss)<<fmt->Bshift;
						}
						else
						{
							px = src[i];
							red = (PIXR(px)>>fmt->Rloss)<<fmt->Rshift;
							green = (PIXG(px)>>fmt->Gloss)<<fmt->Gshift;
							blue = (PIXB(px)>>fmt->Bloss)<<fmt->Bshift;
						}
						dst[i*2] = red|green|blue;
						dst[i*2+1] = red|green|blue;
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
				}
				src+=pitch;
			}
		}
		else
		{
			for (j=0; j<h; j++)
			{
				for (k=0; k<currentScale; k++)
				{
					for (i=0; i<w; i++)
					{
						px = src[i];
						if (depth3d)
						{
							lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
							nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
							int redshift = PIXB(lastpx) + PIXG(lastpx);
							if (redshift > 255)
								redshift = 255;
							int blueshift = PIXR(nextpx) + PIXG(nextpx);
							if (blueshift > 255)
								blueshift = 255;
							px = PIXRGB((int)(PIXR(lastpx)*.69f+redshift*.3f), (int)(PIXG(nextpx)*.3f), (int)(PIXB(nextpx)*.69f+blueshift*.3f));
						}
						dst[i*2] = px;
						dst[i*2+1] = px;
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
				}
				src+=pitch;
			}
		}
		if (SDL_MUSTLOCK(sdl_scrn))
			SDL_UnlockSurface(sdl_scrn);
		SDL_UpdateRect(sdl_scrn,0,0,0,0);
	}
}
#endif

int SDLOpen()
{
#if defined(WIN) && defined(WINCONSOLE)
	FILE * console = fopen("CON", "w" );
#endif
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 1;
	}
	const SDL_VideoInfo * vidInfo = SDL_GetVideoInfo();
	desktopWidth = vidInfo->current_w;
	desktopHeight = vidInfo->current_h;
	SDL_EnableUNICODE(1);
#if defined(WIN) && defined(WINCONSOLE)
	//On Windows, SDL redirects stdout to stdout.txt, which can be annoying when debugging, here we redirect back to the console
	if (console)
	{
		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
		//fclose(console);
	}
#endif
#ifdef WIN
	SDL_SysWMinfo SysInfo;
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWMInfo(&SysInfo) <= 0) {
	    printf("%s : %p\n", SDL_GetError(), SysInfo.window);
	    exit(-1);
	}
	HWND WindowHandle = SysInfo.window;

	// Use GetModuleHandle to get the Exe HMODULE/HINSTANCE
	HMODULE hModExe = GetModuleHandle(NULL);
	HICON hIconSmall = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIconBig = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#elif defined(LIN)
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom((void*)app_icon, 48, 48, 24, 144, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
	SDL_WM_SetIcon(icon, (Uint8*)app_icon_bitmap);
	SDL_FreeSurface(icon);
#endif

	SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	atexit(SDL_Quit);

	return 0;
}

SDL_Surface * SDLSetScreen(int newScale, bool newFullscreen)
{
	scale = newScale;
	fullscreen = newFullscreen;
	SDL_Surface * surface;
#ifndef OGLI
	surface = SDL_SetVideoMode(WINDOWW * newScale, WINDOWH * newScale, 32, SDL_SWSURFACE | (newFullscreen?SDL_FULLSCREEN:0));
#else
	surface = SDL_SetVideoMode(WINDOWW * newScale, WINDOWH * newScale, 32, SDL_OPENGL | SDL_RESIZABLE | (newFullscreen?SDL_FULLSCREEN:0));
#endif
	return surface;
}

void SetCursorEnabled(int enabled)
{
	SDL_ShowCursor(enabled);
}

unsigned int GetTicks()
{
	return SDL_GetTicks();
}

std::map<std::string, std::string> readArguments(int argc, char * argv[])
{
	std::map<std::string, std::string> arguments;

	//Defaults
	arguments["scale"] = "";
	arguments["proxy"] = "";
	arguments["nohud"] = "false"; //the nohud, sound, and scripts commands currently do nothing.
	arguments["sound"] = "false";
	arguments["kiosk"] = "false";
	arguments["scripts"] = "false";
	arguments["open"] = "";
	arguments["ddir"] = "";
	arguments["ptsave"] = "";

	for (int i=1; i<argc; i++)
	{
		if (!strncmp(argv[i], "scale:", 6) && argv[i]+6)
		{
			arguments["scale"] = std::string(argv[i]+6);
		}
		else if (!strncmp(argv[i], "proxy:", 6))
		{
			if(argv[i]+6)
				arguments["proxy"] =  std::string(argv[i]+6);
			else
				arguments["proxy"] = "false";
		}
		else if (!strncmp(argv[i], "nohud", 5))
		{
			arguments["nohud"] = "true";
		}
		else if (!strncmp(argv[i], "kiosk", 5))
		{
			arguments["kiosk"] = "true";
		}
		else if (!strncmp(argv[i], "sound", 5))
		{
			arguments["sound"] = "true";
		}
		else if (!strncmp(argv[i], "scripts", 8))
		{
			arguments["scripts"] = "true";
		}
		else if (!strncmp(argv[i], "open", 5) && i+1<argc)
		{
			arguments["open"] = std::string(argv[i+1]);;
			i++;
		}
		else if (!strncmp(argv[i], "ddir", 5) && i+1<argc)
		{
			arguments["ddir"] = std::string(argv[i+1]);
			i++;
		}
		else if (!strncmp(argv[i], "ptsave", 7) && i+1<argc)
		{
			arguments["ptsave"] = std::string(argv[i+1]);
			i++;
			break;
		}
	}
	return arguments;
}

SDLKey MapNumpad(SDLKey key)
{
	switch(key)
	{
	case SDLK_KP8:
		return SDLK_UP;
	case SDLK_KP2:
		return SDLK_DOWN;
	case SDLK_KP6:
		return SDLK_RIGHT;
	case SDLK_KP4:
		return SDLK_LEFT;
	case SDLK_KP7:
		return SDLK_HOME;
	case SDLK_KP1:
		return SDLK_END;
	case SDLK_KP_PERIOD:
		return SDLK_DELETE;
	case SDLK_KP0:
	case SDLK_KP9:
	case SDLK_KP3:
		return SDLK_UNKNOWN;
	default:
		return key;
	}
}

int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
unsigned int lastTick = 0;
float fps = 0, delta = 1.0f, inputScale = 1.0f;
ui::Engine * engine = NULL;
bool showDoubleScreenDialog = false;
float currentWidth, currentHeight;

void EventProcess(SDL_Event event)
{
	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
	{
		if (event.key.keysym.unicode==0)
		{
			// If unicode is zero, this could be a numpad key with numlock off, or numlock on and shift on (unicode is set to 0 by SDL or the OS in these circumstances. If numlock is on, unicode is the relevant digit character).
			// For some unknown reason, event.key.keysym.mod seems to be unreliable on some computers (keysum.mod&KEY_MOD_NUM is opposite to the actual value), so check keysym.unicode instead.
			// Note: unicode is always zero for SDL_KEYUP events, so this translation won't always work properly for keyup events.
			SDLKey newKey = MapNumpad(event.key.keysym.sym);
			if (newKey != event.key.keysym.sym)
			{
				event.key.keysym.sym = newKey;
				event.key.keysym.unicode = 0;
			}
		}
	}
	switch (event.type)
	{
	case SDL_QUIT:
		if (engine->GetFastQuit() || engine->CloseWindow())
			engine->Exit();
		break;
	case SDL_KEYDOWN:
		engine->onKeyPress(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_KEYUP:
		engine->onKeyRelease(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_MOUSEMOTION:
		engine->onMouseMove(event.motion.x*inputScale, event.motion.y*inputScale);
		mousex = event.motion.x*inputScale;
		mousey = event.motion.y*inputScale;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_WHEELUP)
		{
			engine->onMouseWheel(event.motion.x*inputScale, event.motion.y*inputScale, 1);
		}
		else if (event.button.button == SDL_BUTTON_WHEELDOWN)
		{
			engine->onMouseWheel(event.motion.x*inputScale, event.motion.y*inputScale, -1);
		}
		else
		{
			engine->onMouseClick(event.motion.x*inputScale, event.motion.y*inputScale, event.button.button);
		}
		mousex = event.motion.x*inputScale;
		mousey = event.motion.y*inputScale;
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button != SDL_BUTTON_WHEELUP && event.button.button != SDL_BUTTON_WHEELDOWN)
			engine->onMouseUnclick(event.motion.x*inputScale, event.motion.y*inputScale, event.button.button);
		mousex = event.motion.x*inputScale;
		mousey = event.motion.y*inputScale;
		break;
#ifdef OGLI
	case SDL_VIDEORESIZE:
	{
		float ratio = (float)WINDOWW / WINDOWH;
		float width = event.resize.w;
		float height = width/ratio;

		sdl_scrn = SDL_SetVideoMode(event.resize.w, height, 32, SDL_OPENGL | SDL_RESIZABLE);

		glViewport(0, 0, width, height);
		engine->g->Reset();
		//glScaled(width/currentWidth, height/currentHeight, 1.0f);

		currentWidth = width;
		currentHeight = height;
		inputScale = (float)WINDOWW/currentWidth;

		glLineWidth(currentWidth/(float)WINDOWW);
		if(sdl_scrn == NULL)
		{
			std::cerr << "Oh bugger" << std::endl;
		}
		break;
	}
#endif
#if defined (USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	case SDL_SYSWMEVENT:
		if (event.syswm.msg->subsystem != SDL_SYSWM_X11)
			break;
		sdl_wminfo.info.x11.lock_func();
		XEvent xe = event.syswm.msg->event.xevent;
		if (xe.type==SelectionClear)
		{
			clipboardText = "";
		}
		else if (xe.type==SelectionRequest)
		{
			XEvent xr;
			xr.xselection.type = SelectionNotify;
			xr.xselection.requestor = xe.xselectionrequest.requestor;
			xr.xselection.selection = xe.xselectionrequest.selection;
			xr.xselection.target = xe.xselectionrequest.target;
			xr.xselection.property = xe.xselectionrequest.property;
			xr.xselection.time = xe.xselectionrequest.time;
			if (xe.xselectionrequest.target==XA_TARGETS)
			{
				// send list of supported formats
				Atom targets[] = {XA_TARGETS, XA_STRING, XA_UTF8_STRING};
				xr.xselection.property = xe.xselectionrequest.property;
				XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, (int)(sizeof(targets)/sizeof(Atom)));
			}
			// TODO: Supporting more targets would be nice
			else if ((xe.xselectionrequest.target==XA_STRING || xe.xselectionrequest.target==XA_UTF8_STRING))
			{
				XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, xe.xselectionrequest.target, 8, PropModeReplace, (unsigned char*)clipboardText.c_str(), clipboardText.size()+1);
			}
			else
			{
				// refuse clipboard request
				xr.xselection.property = None;
			}
			XSendEvent(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, 0, 0, &xr);
		}
		sdl_wminfo.info.x11.unlock_func();
#endif
	}
}

void DoubleScreenDialog()
{
	std::stringstream message;
	message << "Switching to double size mode since your screen was determined to be large enough: ";
	message << desktopWidth << "x" << desktopHeight << " detected, " << WINDOWW*2 << "x" << WINDOWH*2 << " required";
	message << "\nTo undo this, hit Cancel. You can toggle double size mode in settings at any time.";
	if (!ConfirmPrompt::Blocking("Large screen detected", message.str()))
	{
		Client::Ref().SetPref("Scale", 1);
		engine->SetScale(1);
#ifdef WIN
		LoadWindowPosition(1);
#endif
	}
}

void EngineProcess()
{
	double frameTimeAvg = 0.0f, correctedFrameTimeAvg = 0.0f;
	SDL_Event event;
	while(engine->Running())
	{
		int frameStart = SDL_GetTicks();
		if(engine->Broken()) { engine->UnBreak(); break; }
		event.type = 0;
		while (SDL_PollEvent(&event))
		{
			EventProcess(event);
			event.type = 0; //Clear last event
		}
		if(engine->Broken()) { engine->UnBreak(); break; }

		engine->Tick();
		engine->Draw();
		
		if(scale != engine->Scale || fullscreen != engine->Fullscreen)
		{
			sdl_scrn = SDLSetScreen(engine->Scale, engine->Fullscreen);
			inputScale = 1.0f/(float)scale;
		}

#ifdef OGLI
		blit();
#else
		if(engine->Scale==2)
			blit2(engine->g->vid, engine->Scale);
		else
			blit(engine->g->vid);
#endif

		int frameTime = SDL_GetTicks() - frameStart;
		frameTimeAvg = frameTimeAvg * 0.8 + frameTime * 0.2;
		int fpsLimit = ui::Engine::Ref().FpsLimit;
		if(fpsLimit > 2)
		{
			double offset = 1000.0 / fpsLimit - frameTimeAvg;
			if(offset > 0)
				SDL_Delay(offset + 0.5);
		}
		int correctedFrameTime = SDL_GetTicks() - frameStart;
		correctedFrameTimeAvg = correctedFrameTimeAvg * 0.95 + correctedFrameTime * 0.05;
		engine->SetFps(1000.0 / correctedFrameTimeAvg);
		if(frameStart - lastTick > 1000)
		{
			//Run client tick every second
			lastTick = frameStart;
			Client::Ref().Tick();
		}
		if (showDoubleScreenDialog)
		{
			showDoubleScreenDialog = false;
			DoubleScreenDialog();
		}
	}
#ifdef DEBUG
	std::cout << "Breaking out of EngineProcess" << std::endl;
#endif
}

int GetModifiers()
{
	return SDL_GetModState();
}

#ifdef WIN

// Returns true if the loaded position was set
// Returns false if something went wrong: SDL_GetWMInfo failed or the loaded position was invalid
bool LoadWindowPosition(int scale)
{
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		int windowW = WINDOWW * scale;
		int windowH = WINDOWH * scale;

		int savedWindowX = Client::Ref().GetPrefInteger("WindowX", INT_MAX);
		int savedWindowY = Client::Ref().GetPrefInteger("WindowY", INT_MAX);
		
		// Center the window on the primary desktop by default
		int newWindowX = (desktopWidth - windowW) / 2;
		int newWindowY = (desktopHeight - windowH) / 2;

		bool success = false;

		if (savedWindowX != INT_MAX && savedWindowY != INT_MAX)
		{
			POINT windowPoints[] = {
				{savedWindowX, savedWindowY},                       // Top-left
				{savedWindowX + windowW, savedWindowY + windowH}    // Bottom-right
			};

			MONITORINFO monitor;
			monitor.cbSize = sizeof(monitor);
			if (GetMonitorInfo(MonitorFromPoint(windowPoints[0], MONITOR_DEFAULTTONEAREST), &monitor) != 0)
			{
				// Only use the saved window position if it lies inside the visible screen
				if (PtInRect(&monitor.rcMonitor, windowPoints[0]) && PtInRect(&monitor.rcMonitor, windowPoints[1]))
				{
					newWindowX = savedWindowX;
					newWindowY = savedWindowY;

					success = true;
				}
				else
				{
					// Center the window on the nearest monitor
					newWindowX = monitor.rcMonitor.left + (monitor.rcMonitor.right - monitor.rcMonitor.left - windowW) / 2;
					newWindowY = monitor.rcMonitor.top + (monitor.rcMonitor.bottom - monitor.rcMonitor.top - windowH) / 2;
				}
			}
		}
		
		SetWindowPos(sysInfo.window, 0, newWindowX, newWindowY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

		// True if we didn't use the default, i.e. the position was valid
		return success;
	}

	return false;
}

// Returns true if the window position was saved
bool SaveWindowPosition()
{
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		WINDOWPLACEMENT placement;
		placement.length = sizeof(placement);
		GetWindowPlacement(sysInfo.window, &placement);

		Client::Ref().SetPref("WindowX", (int)placement.rcNormalPosition.left);
		Client::Ref().SetPref("WindowY", (int)placement.rcNormalPosition.top);

		return true;
	}

	return false;
}

#endif

void BlueScreen(const char * detailMessage){
	ui::Engine * engine = &ui::Engine::Ref();
	engine->g->fillrect(0, 0, engine->GetWidth(), engine->GetHeight(), 17, 114, 169, 210);

	std::string errorTitle = "ERROR";
	std::string errorDetails = "Details: " + std::string(detailMessage);
	std::string errorHelp = "An unrecoverable fault has occurred, please report the error by visiting the website below\n"
		"http://" SERVER;
	int currentY = 0, width, height;
	int errorWidth = 0;
	Graphics::textsize(errorHelp.c_str(), errorWidth, height);
	
	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorTitle.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle.c_str(), width, height);
	currentY += height + 4;

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorDetails.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle.c_str(), width, height);
	currentY += height + 4;

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorHelp.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle.c_str(), width, height);
	currentY += height + 4;
	
	//Death loop
	SDL_Event event;	
	while(true)
	{
		while (SDL_PollEvent(&event))
			if(event.type == SDL_QUIT)
				exit(-1);
#ifdef OGLI
		blit();
#else
		if(engine->Scale==2)
			blit2(engine->g->vid, engine->Scale);
		else
			blit(engine->g->vid);
#endif
	}
}

void SigHandler(int signal)
{
	switch(signal){
	case SIGSEGV:
		BlueScreen("Memory read/write error");
		break;
	case SIGFPE:
		BlueScreen("Floating point exception");
		break;
	case SIGILL:
		BlueScreen("Program execution exception");
		break;
	case SIGABRT:
		BlueScreen("Unexpected program abort");
		break;
	}
}

int main(int argc, char * argv[])
{
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif
	currentWidth = WINDOWW; 
	currentHeight = WINDOWH;


	std::map<std::string, std::string> arguments = readArguments(argc, argv);

	if(arguments["ddir"].length())
#ifdef WIN
		_chdir(arguments["ddir"].c_str());
#else
		chdir(arguments["ddir"].c_str());
#endif

	int tempScale = 1;
	bool tempFullscreen = false;

	tempScale = Client::Ref().GetPrefInteger("Scale", 1);
	tempFullscreen = Client::Ref().GetPrefBool("Fullscreen", false);


	if(arguments["kiosk"] == "true")
	{
		tempFullscreen = true;
		Client::Ref().SetPref("Fullscreen", tempFullscreen);
	}

	if(arguments["scale"].length())
	{
		tempScale = format::StringToNumber<int>(arguments["scale"]);
		Client::Ref().SetPref("Scale", tempScale);
	}

	std::string proxyString = "";
	if(arguments["proxy"].length())
	{
		if(arguments["proxy"] == "false")
		{
			proxyString = "";
			Client::Ref().SetPref("Proxy", "");	
		}
		else
		{
			proxyString = (arguments["proxy"]);
			Client::Ref().SetPref("Proxy", arguments["proxy"]);
		}
	}
	else if(Client::Ref().GetPrefString("Proxy", "").length())
	{
		proxyString = (Client::Ref().GetPrefString("Proxy", ""));
	}

	Client::Ref().Initialise(proxyString);

	if(tempScale != 1 && tempScale != 2)
		tempScale = 1;

	SDLOpen();
	if (Client::Ref().IsFirstRun() && desktopWidth > WINDOWW*2+50 && desktopHeight > WINDOWH*2+50)
	{
		tempScale = 2;
		Client::Ref().SetPref("Scale", 2);
		showDoubleScreenDialog = true;
	}
#ifdef WIN
	LoadWindowPosition(tempScale);
#endif
	sdl_scrn = SDLSetScreen(tempScale, tempFullscreen);

#ifdef OGLI
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	//glScaled(2.0f, 2.0f, 1.0f);
#endif
#if defined(OGLI) && !defined(MACOSX)
	int status = glewInit();
	if(status != GLEW_OK)
	{
		fprintf(stderr, "Initializing Glew: %d\n", status);
		exit(-1);
	}
#endif
#if defined (USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_VERSION(&sdl_wminfo.version);
	if(SDL_GetWMInfo(&sdl_wminfo) > 0)
	{
		sdl_wminfo.info.x11.lock_func();
		XA_CLIPBOARD = XInternAtom(sdl_wminfo.info.x11.display, "CLIPBOARD", 1);
		XA_TARGETS = XInternAtom(sdl_wminfo.info.x11.display, "TARGETS", 1);
		XA_UTF8_STRING = XInternAtom(sdl_wminfo.info.x11.display, "UTF8_STRING", 1);
		sdl_wminfo.info.x11.unlock_func();
	} 
	else
	{
		fprintf(stderr, "X11 setup failed, X11 window info not found");
		exit(-1);
	}
#endif
	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().Scale = scale;
	inputScale = 1.0f/float(scale);
	ui::Engine::Ref().Fullscreen = fullscreen;

	engine = &ui::Engine::Ref();
	engine->SetMaxSize(desktopWidth, desktopHeight);
	engine->Begin(WINDOWW, WINDOWH);
	engine->SetFastQuit(Client::Ref().GetPrefBool("FastQuit", true));

#if !defined(DEBUG) && !defined(_DEBUG)
	//Get ready to catch any dodgy errors
	signal(SIGSEGV, SigHandler);
	signal(SIGFPE, SigHandler);
	signal(SIGILL, SigHandler);
	signal(SIGABRT, SigHandler);
#endif

#ifdef X86_SSE
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif
#ifdef X86_SSE3
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif

	GameController * gameController = NULL;
#if !defined(DEBUG) && !defined(_DEBUG)
	try {
#endif

		gameController = new GameController();
		engine->ShowWindow(gameController->GetView());

		if(arguments["open"].length())
		{
#ifdef DEBUG
			std::cout << "Loading " << arguments["open"] << std::endl;
#endif
			if(Client::Ref().FileExists(arguments["open"]))
			{
				try
				{
					std::vector<unsigned char> gameSaveData = Client::Ref().ReadFile(arguments["open"]);
					if(!gameSaveData.size())
					{
						new ErrorMessage("Error", "Could not read file");
					}
					else
					{
						SaveFile * newFile = new SaveFile(arguments["open"]);
						GameSave * newSave = new GameSave(gameSaveData);
						newFile->SetGameSave(newSave);
						gameController->LoadSaveFile(newFile);
						delete newFile;
					}

				}
				catch(std::exception & e)
				{
					new ErrorMessage("Error", "Could not open save file:\n"+std::string(e.what())) ;
				}
			}
			else
			{
				new ErrorMessage("Error", "Could not open file");
			}
		}

		if(arguments["ptsave"].length())
		{
			engine->g->fillrect((engine->GetWidth()/2)-101, (engine->GetHeight()/2)-26, 202, 52, 0, 0, 0, 210);
			engine->g->drawrect((engine->GetWidth()/2)-100, (engine->GetHeight()/2)-25, 200, 50, 255, 255, 255, 180);
			engine->g->drawtext((engine->GetWidth()/2)-(Graphics::textwidth("Loading save...")/2), (engine->GetHeight()/2)-5, "Loading save...", style::Colour::InformationTitle.Red, style::Colour::InformationTitle.Green, style::Colour::InformationTitle.Blue, 255);

#ifdef OGLI
			blit();
#else
			if(engine->Scale==2)
				blit2(engine->g->vid, engine->Scale);
			else
				blit(engine->g->vid);
#endif
			std::string ptsaveArg = arguments["ptsave"];
			try
			{
				if (ptsaveArg.find("ptsave:"))
					throw std::runtime_error("Invalid save link");

				std::string saveIdPart = "";
				int saveId;
				size_t hashPos = ptsaveArg.find('#');
				if (hashPos != std::string::npos)
				{
					saveIdPart = ptsaveArg.substr(7, hashPos-7);
				}
				else
				{
					saveIdPart = ptsaveArg.substr(7);
				}
				if (!saveIdPart.length())
					throw std::runtime_error("No Save ID");
#ifdef DEBUG
				std::cout << "Got Ptsave: id: " <<  saveIdPart << std::endl;
#endif
				saveId = format::StringToNumber<int>(saveIdPart);
				if (!saveId)
					throw std::runtime_error("Invalid Save ID");

				SaveInfo * newSave = Client::Ref().GetSave(saveId, 0);
				if (!newSave)
					throw std::runtime_error("Could not load save info");
				std::vector<unsigned char> saveData = Client::Ref().GetSaveData(saveId, 0);
				if (!saveData.size())
					throw std::runtime_error("Could not load save\n" + Client::Ref().GetLastError());
				GameSave * newGameSave = new GameSave(saveData);
				newSave->SetGameSave(newGameSave);

				gameController->LoadSave(newSave);
				delete newSave;
			}
			catch (std::exception & e)
			{
				new ErrorMessage("Error", e.what());
			}
		}

		//initial mouse coords
		int sdl_x, sdl_y;
		SDL_GetMouseState(&sdl_x, &sdl_y);
		engine->onMouseMove(sdl_x*inputScale, sdl_y*inputScale);
		EngineProcess();
		
#ifdef WIN
		SaveWindowPosition();
#endif

#if !defined(DEBUG) && !defined(_DEBUG)
	}
	catch(exception& e)
	{
		BlueScreen(e.what());
	}
#endif
	
	Client::Ref().SetPref("Scale", ui::Engine::Ref().GetScale());
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
	Client::Ref().Shutdown();
	return 0;
}

#endif
