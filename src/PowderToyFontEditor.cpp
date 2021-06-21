#include "Config.h"
#include <ctime>
#include <climits>
#ifdef WIN
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
#if defined(LIN)
#include "icon.h"
#endif
#include <stdexcept>

#ifndef WIN
#include <unistd.h>
#endif
#ifdef MACOSX
# include "common/macosx.h"
#endif

#include "Format.h"
#include "Misc.h"

#include "graphics/Graphics.h"

#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/Client.h"

#include "gui/game/GameController.h"
#include "gui/game/GameView.h"
#include "gui/font/FontEditor.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"
#include "gui/interface/Engine.h"

#define INCLUDE_SYSWM
#include "SDLCompat.h"

int desktopWidth = 1280, desktopHeight = 1024;

SDL_Window * sdl_window;
SDL_Renderer * sdl_renderer;
SDL_Texture * sdl_texture;
int scale = 1;
bool fullscreen = false;
bool altFullscreen = false;
bool forceIntegerScaling = true;
bool resizable = false;


void StartTextInput()
{
	SDL_StartTextInput();
}

void StopTextInput()
{
	SDL_StopTextInput();
}

void SetTextInputRect(int x, int y, int w, int h)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_SetTextInputRect(&rect);
}

void ClipboardPush(ByteString text)
{
	SDL_SetClipboardText(text.c_str());
}

ByteString ClipboardPull()
{
	return ByteString(SDL_GetClipboardText());
}

int GetModifiers()
{
	return SDL_GetModState();
}

void CalculateMousePosition(int *x, int *y)
{
	int globalMx, globalMy;
	SDL_GetGlobalMouseState(&globalMx, &globalMy);
	int windowX, windowY;
	SDL_GetWindowPosition(sdl_window, &windowX, &windowY);

	if (x)
		*x = (globalMx - windowX) / scale;
	if (y)
		*y = (globalMy - windowY) / scale;
}

#ifdef OGLI
void blit()
{
	SDL_GL_SwapBuffers();
}
#else
void blit(pixel * vid)
{
	SDL_UpdateTexture(sdl_texture, NULL, vid, WINDOWW * sizeof (Uint32));
	// need to clear the renderer if there are black edges (fullscreen, or resizable window)
	if (fullscreen || resizable)
		SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
}
#endif

void RecreateWindow();
int SDLOpen()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	RecreateWindow();

	int displayIndex = SDL_GetWindowDisplayIndex(sdl_window);
	if (displayIndex >= 0)
	{
		SDL_Rect rect;
		if (!SDL_GetDisplayUsableBounds(displayIndex, &rect))
		{
			desktopWidth = rect.w;
			desktopHeight = rect.h;
		}
	}

#ifdef WIN
	SDL_SysWMinfo SysInfo;
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWindowWMInfo(sdl_window, &SysInfo) <= 0)
	{
	    printf("%s : %p\n", SDL_GetError(), SysInfo.info.win.window);
	    exit(-1);
	}
	HWND WindowHandle = SysInfo.info.win.window;

	// Use GetModuleHandle to get the Exe HMODULE/HINSTANCE
	HMODULE hModExe = GetModuleHandle(NULL);
	HICON hIconSmall = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIconBig = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#endif
#ifdef LIN
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom((void*)app_icon, 128, 128, 32, 512, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_SetWindowIcon(sdl_window, icon);
	SDL_FreeSurface(icon);
#endif

	return 0;
}

void SDLSetScreen(int scale_, bool resizable_, bool fullscreen_, bool altFullscreen_, bool forceIntegerScaling_)
{
//	bool changingScale = scale != scale_;
	bool changingFullscreen = fullscreen_ != fullscreen || (altFullscreen_ != altFullscreen && fullscreen);
	bool changingResizable = resizable != resizable_;
	scale = scale_;
	fullscreen = fullscreen_;
	altFullscreen = altFullscreen_;
	resizable = resizable_;
	forceIntegerScaling = forceIntegerScaling_;
	// Recreate the window when toggling fullscreen, due to occasional issues
	// Also recreate it when enabling resizable windows, to fix bugs on windows,
	//  see https://github.com/jacob1/The-Powder-Toy/issues/24
	if (changingFullscreen || (changingResizable && resizable && !fullscreen))
	{
		RecreateWindow();
		return;
	}
	if (changingResizable)
		SDL_RestoreWindow(sdl_window);

	SDL_SetWindowSize(sdl_window, WINDOWW * scale, WINDOWH * scale);
	SDL_RenderSetIntegerScale(sdl_renderer, forceIntegerScaling && fullscreen ? SDL_TRUE : SDL_FALSE);
	unsigned int flags = 0;
	if (fullscreen)
		flags = altFullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(sdl_window, flags);
	if (fullscreen)
		SDL_RaiseWindow(sdl_window);
	SDL_SetWindowResizable(sdl_window, resizable ? SDL_TRUE : SDL_FALSE);
}

void RecreateWindow()
{
	unsigned int flags = 0;
	if (fullscreen)
		flags = altFullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (resizable && !fullscreen)
		flags |= SDL_WINDOW_RESIZABLE;

	if (sdl_texture)
		SDL_DestroyTexture(sdl_texture);
	if (sdl_renderer)
		SDL_DestroyRenderer(sdl_renderer);
	if (sdl_window)
	{
		SDL_DestroyWindow(sdl_window);
	}

	sdl_window = SDL_CreateWindow("The Powder Toy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWW * scale, WINDOWH * scale,
	                              flags);
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
	SDL_RenderSetLogicalSize(sdl_renderer, WINDOWW, WINDOWH);
	if (forceIntegerScaling && fullscreen)
		SDL_RenderSetIntegerScale(sdl_renderer, SDL_TRUE);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOWW, WINDOWH);
	SDL_RaiseWindow(sdl_window);
	//Uncomment this to enable resizing
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//SDL_SetWindowResizable(sdl_window, SDL_TRUE);
}

unsigned int GetTicks()
{
	return SDL_GetTicks();
}

int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
unsigned int lastTick = 0;
unsigned int lastFpsUpdate = 0;
float fps = 0;
ui::Engine * engine = NULL;
bool showDoubleScreenDialog = false;
float currentWidth, currentHeight;

int mousex = 0, mousey = 0;
int mouseButton = 0;
bool mouseDown = false;

bool calculatedInitialMouse = false, delay = false;
bool hasMouseMoved = false;

void EventProcess(SDL_Event event)
{
	switch (event.type)
	{
	case SDL_QUIT:
		if (engine->GetFastQuit() || engine->CloseWindow())
			engine->Exit();
		break;
	case SDL_KEYDOWN:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		if (!event.key.repeat && event.key.keysym.sym == 'q' && (event.key.keysym.mod&KMOD_CTRL))
			engine->ConfirmExit();
		else
			engine->onKeyPress(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_KEYUP:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine->onKeyRelease(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_TEXTINPUT:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine->onTextInput(ByteString(event.text.text).FromUtf8());
		break;
	case SDL_TEXTEDITING:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine->onTextEditing(ByteString(event.edit.text).FromUtf8(), event.edit.start);
		break;
	case SDL_MOUSEWHEEL:
	{
		int x = event.wheel.x;
		int y = event.wheel.y;
		if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
		{
			x *= -1;
			y *= -1;
		}

		engine->onMouseWheel(mousex, mousey, y); // TODO: pass x?
		break;
	}
	case SDL_MOUSEMOTION:
		mousex = event.motion.x;
		mousey = event.motion.y;
		engine->onMouseMove(mousex, mousey);

		hasMouseMoved = true;
		break;
	case SDL_DROPFILE:
		engine->onFileDrop(event.drop.file);
		SDL_free(event.drop.file);
		break;
	case SDL_MOUSEBUTTONDOWN:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			mousex = event.motion.x;
			mousey = event.motion.y;
		}
		mouseButton = event.button.button;
		engine->onMouseClick(event.motion.x, event.motion.y, mouseButton);

		mouseDown = true;
#if !defined(NDEBUG) && !defined(DEBUG)
		SDL_CaptureMouse(SDL_TRUE);
#endif
		break;
	case SDL_MOUSEBUTTONUP:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			mousex = event.motion.x;
			mousey = event.motion.y;
		}
		mouseButton = event.button.button;
		engine->onMouseUnclick(mousex, mousey, mouseButton);

		mouseDown = false;
#if !defined(NDEBUG) && !defined(DEBUG)
		SDL_CaptureMouse(SDL_FALSE);
#endif
		break;
	case SDL_WINDOWEVENT:
	{
		switch (event.window.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
			if (!calculatedInitialMouse)
			{
				//initial mouse coords, sdl won't tell us this if mouse hasn't moved
				CalculateMousePosition(&mousex, &mousey);
				engine->onMouseMove(mousex, mousey);
				calculatedInitialMouse = true;
			}
			break;
		// This event would be needed in certain glitchy cases of window resizing
		// But for all currently tested cases, it isn't needed
		/*case SDL_WINDOWEVENT_RESIZED:
		{
			float width = event.window.data1;
			float height = event.window.data2;

			currentWidth = width;
			currentHeight = height;
			// this "* scale" thing doesn't really work properly
			// currently there is a bug where input doesn't scale properly after resizing, only when double scale mode is active
			inputScaleH = (float)WINDOWW * scale / currentWidth;
			inputScaleV = (float)WINDOWH * scale / currentHeight;
			std::cout << "Changing input scale to " << inputScaleH << "x" << inputScaleV << std::endl;
			break;
		}*/
		// This would send a mouse up event when focus is lost
		// Not even sdl itself will know when the mouse was released if it happens in another window
		// So it will ignore the next mouse down (after tpt is re-focused) and not send any events at all
		// This is more unintuitive than pretending the mouse is still down when it's not, so this code is commented out
		/*case SDL_WINDOWEVENT_FOCUS_LOST:
			if (mouseDown)
			{
				mouseDown = false;
				engine->onMouseUnclick(mousex, mousey, mouseButton);
			}
			break;*/
		}
		break;
	}
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

		if (scale != engine->Scale || fullscreen != engine->Fullscreen ||
				altFullscreen != engine->GetAltFullscreen() ||
				forceIntegerScaling != engine->GetForceIntegerScaling() || resizable != engine->GetResizable())
		{
			SDLSetScreen(engine->Scale, engine->GetResizable(), engine->Fullscreen, engine->GetAltFullscreen(),
						 engine->GetForceIntegerScaling());
		}

#ifdef OGLI
		blit();
#else
		blit(engine->g->vid);
#endif

		int frameTime = SDL_GetTicks() - frameStart;
		frameTimeAvg = frameTimeAvg * 0.8 + frameTime * 0.2;
		float fpsLimit = ui::Engine::Ref().FpsLimit;
		if(fpsLimit > 2)
		{
			double offset = 1000.0 / fpsLimit - frameTimeAvg;
			if(offset > 0)
				SDL_Delay(Uint32(offset + 0.5));
		}
		int correctedFrameTime = SDL_GetTicks() - frameStart;
		correctedFrameTimeAvg = correctedFrameTimeAvg * 0.95 + correctedFrameTime * 0.05;
		if (frameStart - lastFpsUpdate > 200)
		{
			engine->SetFps(1000.0 / correctedFrameTimeAvg);
			lastFpsUpdate = frameStart;
		}
		if (frameStart - lastTick > 100)
		{
			lastTick = frameStart;
		}
		if (showDoubleScreenDialog)
		{
			showDoubleScreenDialog = false;
		}
	}
#ifdef DEBUG
	std::cout << "Breaking out of EngineProcess" << std::endl;
#endif
}

#ifdef main
# undef main // thank you sdl
#endif

int main(int argc, char * argv[])
{
	currentWidth = WINDOWW;
	currentHeight = WINDOWH;
	
	scale = 1;
	if (argc >= 3)
	{
		std::istringstream ss(argv[2]);
		int buf;
		if (ss >> buf)
		{
			scale = buf;
		}
	}
	resizable = false;
	fullscreen = false;
	altFullscreen = false;
	forceIntegerScaling = true;

	// TODO: maybe bind the maximum allowed scale to screen size somehow
	if(scale < 1 || scale > 10)
		scale = 1;

	SDLOpen();

	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().Scale = scale;
	ui::Engine::Ref().SetResizable(resizable);
	ui::Engine::Ref().Fullscreen = fullscreen;
	ui::Engine::Ref().SetAltFullscreen(altFullscreen);
	ui::Engine::Ref().SetForceIntegerScaling(forceIntegerScaling);

	engine = &ui::Engine::Ref();
	engine->SetMaxSize(desktopWidth, desktopHeight);
	engine->Begin(WINDOWW, WINDOWH);
	engine->SetFastQuit(true);

	GameController * gameController = NULL;

	if (argc >= 2)
	{
		engine->ShowWindow(new FontEditor(argv[1]));
	}
	else
	{
		std::cerr << "path to font.cpp not supplied" << std::endl;
		exit(1);
	}

	EngineProcess();
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
	if (SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_OPENGL)
	{
		// * nvidia-460 egl registers callbacks with x11 that end up being called
		//   after egl is unloaded unless we grab it here and release it after
		//   sdl closes the display. this is an nvidia driver weirdness but
		//   technically an sdl bug. glfw has this fixed:
		//   https://github.com/glfw/glfw/commit/9e6c0c747be838d1f3dc38c2924a47a42416c081
		SDL_GL_LoadLibrary(NULL);
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_GL_UnloadLibrary();
	}
	SDL_Quit();
	return 0;
}
