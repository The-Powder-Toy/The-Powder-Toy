#include "PowderToySDL.h"
#include "SimulationConfig.h"
#include "WindowIcon.h"
#include "Config.h"
#include "gui/interface/Engine.h"
#include "graphics/Graphics.h"
#include "common/platform/Platform.h"
#include <iostream>

int desktopWidth = 1280;
int desktopHeight = 1024;
SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;
SDL_Texture *sdl_texture = NULL;
int scale = 1;
bool fullscreen = false;
bool altFullscreen = false;
bool forceIntegerScaling = true;
bool resizable = false;
bool momentumScroll = true;
bool showAvatars = true;
uint64_t lastTick = 0;
uint64_t lastFpsUpdate = 0;
bool showLargeScreenDialog = false;
int mousex = 0;
int mousey = 0;
int mouseButton = 0;
bool mouseDown = false;
bool calculatedInitialMouse = false;
bool hasMouseMoved = false;

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

unsigned int GetTicks()
{
	return SDL_GetTicks();
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

void blit(pixel *vid)
{
	SDL_UpdateTexture(sdl_texture, NULL, vid, WINDOWW * sizeof (Uint32));
	// need to clear the renderer if there are black edges (fullscreen, or resizable window)
	if (fullscreen || resizable)
		SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
}

void SDLOpen()
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Initializing SDL (video subsystem): %s\n", SDL_GetError());
		Platform::Exit(-1);
	}

	if (!RecreateWindow())
	{
		fprintf(stderr, "Creating SDL window: %s\n", SDL_GetError());
		Platform::Exit(-1);
	}

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

	if constexpr (SET_WINDOW_ICON)
	{
		WindowIcon(sdl_window);
	}
}

void SDLClose()
{
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
	if (changingFullscreen || altFullscreen || (changingResizable && resizable && !fullscreen))
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

bool RecreateWindow()
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
		SaveWindowPosition();
		SDL_DestroyWindow(sdl_window);
	}

	sdl_window = SDL_CreateWindow(APPNAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWW * scale, WINDOWH * scale,
	                              flags);
	if (!sdl_window)
	{
		return false;
	}
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
	if (!sdl_renderer)
	{
		fprintf(stderr, "SDL_CreateRenderer failed; available renderers:\n");
		int num = SDL_GetNumRenderDrivers();
		for (int i = 0; i < num; ++i)
		{
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(i, &info);
			fprintf(stderr, " - %s\n", info.name);
		}
		return false;
	}
	SDL_RenderSetLogicalSize(sdl_renderer, WINDOWW, WINDOWH);
	if (forceIntegerScaling && fullscreen)
		SDL_RenderSetIntegerScale(sdl_renderer, SDL_TRUE);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOWW, WINDOWH);
	SDL_RaiseWindow(sdl_window);
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	//Uncomment this to enable resizing
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//SDL_SetWindowResizable(sdl_window, SDL_TRUE);

	LoadWindowPosition();

	return true;
}

void EventProcess(const SDL_Event &event)
{
	auto &engine = ui::Engine::Ref();
	switch (event.type)
	{
	case SDL_QUIT:
		if (engine.GetFastQuit() || engine.CloseWindow())
			engine.Exit();
		break;
	case SDL_KEYDOWN:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		if (!event.key.repeat && event.key.keysym.sym == 'q' && (event.key.keysym.mod&KMOD_CTRL))
			engine.ConfirmExit();
		else
			engine.onKeyPress(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_KEYUP:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine.onKeyRelease(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_TEXTINPUT:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine.onTextInput(ByteString(event.text.text).FromUtf8());
		break;
	case SDL_TEXTEDITING:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine.onTextEditing(ByteString(event.edit.text).FromUtf8(), event.edit.start);
		break;
	case SDL_MOUSEWHEEL:
	{
		// int x = event.wheel.x;
		int y = event.wheel.y;
		if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
		{
			// x *= -1;
			y *= -1;
		}

		engine.onMouseWheel(mousex, mousey, y); // TODO: pass x?
		break;
	}
	case SDL_MOUSEMOTION:
		mousex = event.motion.x;
		mousey = event.motion.y;
		engine.onMouseMove(mousex, mousey);

		hasMouseMoved = true;
		break;
	case SDL_DROPFILE:
		engine.onFileDrop(event.drop.file);
		SDL_free(event.drop.file);
		break;
	case SDL_MOUSEBUTTONDOWN:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			mousex = event.button.x;
			mousey = event.button.y;
		}
		mouseButton = event.button.button;
		engine.onMouseClick(mousex, mousey, mouseButton);

		mouseDown = true;
		if constexpr (!DEBUG)
		{
			SDL_CaptureMouse(SDL_TRUE);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			mousex = event.button.x;
			mousey = event.button.y;
		}
		mouseButton = event.button.button;
		engine.onMouseUnclick(mousex, mousey, mouseButton);

		mouseDown = false;
		if constexpr (!DEBUG)
		{
			SDL_CaptureMouse(SDL_FALSE);
		}
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
				engine.initialMouse(mousex, mousey);
				engine.onMouseMove(mousex, mousey);
				calculatedInitialMouse = true;
			}
			break;
		}
		break;
	}
	}
}

void EngineProcess()
{
	double correctedFrameTimeAvg = 0;
	SDL_Event event;

	uint64_t drawingTimer = 0;
	auto frameStart = uint64_t(SDL_GetTicks()) * UINT64_C(1'000'000);

	auto &engine = ui::Engine::Ref();
	while(engine.Running())
	{
		if(engine.Broken()) { engine.UnBreak(); break; }
		event.type = 0;
		while (SDL_PollEvent(&event))
		{
			EventProcess(event);
			event.type = 0; //Clear last event
		}
		if(engine.Broken()) { engine.UnBreak(); break; }

		engine.Tick();

		int drawcap = ui::Engine::Ref().GetDrawingFrequencyLimit();
		if (!drawcap || drawingTimer > 1e9f / drawcap)
		{
			engine.Draw();
			drawingTimer = 0;

			if (scale != engine.Scale || fullscreen != engine.Fullscreen ||
					altFullscreen != engine.GetAltFullscreen() ||
					forceIntegerScaling != engine.GetForceIntegerScaling() || resizable != engine.GetResizable())
			{
				SDLSetScreen(engine.Scale, engine.GetResizable(), engine.Fullscreen, engine.GetAltFullscreen(),
							 engine.GetForceIntegerScaling());
			}

			blit(engine.g->Data());
		}
		auto fpsLimit = ui::Engine::Ref().FpsLimit;
		auto now = uint64_t(SDL_GetTicks()) * UINT64_C(1'000'000);
		auto oldFrameStart = frameStart;
		frameStart = now;
		if (fpsLimit > 2)
		{
			auto timeBlockDuration = uint64_t(UINT64_C(1'000'000'000) / fpsLimit);
			auto oldFrameStartTimeBlock = oldFrameStart / timeBlockDuration;
			auto frameStartTimeBlock = oldFrameStartTimeBlock + 1U;
			frameStart = std::max(frameStart, frameStartTimeBlock * timeBlockDuration);
			SDL_Delay((frameStart - now) / UINT64_C(1'000'000));
		}
		auto correctedFrameTime = frameStart - oldFrameStart;
		drawingTimer += correctedFrameTime;
		correctedFrameTimeAvg = correctedFrameTimeAvg + (correctedFrameTime - correctedFrameTimeAvg) * 0.05;
		if (frameStart - lastFpsUpdate > UINT64_C(200'000'000))
		{
			engine.SetFps(1e9f / correctedFrameTimeAvg);
			lastFpsUpdate = frameStart;
		}
		if (frameStart - lastTick > UINT64_C(100'000'000))
		{
			lastTick = frameStart;
			TickClient();
		}
		if (showLargeScreenDialog)
		{
			showLargeScreenDialog = false;
			LargeScreenDialog();
		}
	}
	if constexpr (DEBUG)
	{
		std::cout << "Breaking out of EngineProcess" << std::endl;
	}
}
