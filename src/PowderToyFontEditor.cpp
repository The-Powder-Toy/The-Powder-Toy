#include "PowderToySDL.h"
#include "graphics/Graphics.h"
#include "common/Platform.h"
#include "common/tpt-rand.h"
#include "gui/font/FontEditor.h"
#include "gui/interface/Engine.h"
#include "Config.h"
#include "SimulationConfig.h"
#include <iostream>
#include <memory>

void LoadWindowPosition()
{
}

void SaveWindowPosition()
{
}

void LargeScreenDialog()
{
}

void TickClient()
{
}

struct ExplicitSingletons
{
	// These need to be listed in the order they are populated in main.
	std::unique_ptr<RNG> rng;
	std::unique_ptr<ui::Engine> engine;
};
static std::unique_ptr<ExplicitSingletons> explicitSingletons;

int main(int argc, char * argv[])
{
	Platform::SetupCrt();
	atexit([]() {
		ui::Engine::Ref().CloseWindow();
		explicitSingletons.reset();
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
	});
	explicitSingletons = std::make_unique<ExplicitSingletons>();
	
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

	explicitSingletons->rng = std::make_unique<RNG>();
	explicitSingletons->engine = std::make_unique<ui::Engine>();

	SDLOpen();

	StopTextInput();

	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().Scale = scale;
	ui::Engine::Ref().SetResizable(resizable);
	ui::Engine::Ref().Fullscreen = fullscreen;
	ui::Engine::Ref().SetAltFullscreen(altFullscreen);
	ui::Engine::Ref().SetForceIntegerScaling(forceIntegerScaling);

	auto &engine = ui::Engine::Ref();
	engine.SetMaxSize(desktopWidth, desktopHeight);
	engine.Begin(WINDOWW, WINDOWH);
	engine.SetFastQuit(true);

	if (argc >= 2)
	{
		engine.ShowWindow(new FontEditor(argv[1]));
	}
	else
	{
		std::cerr << "path to font.cpp not supplied" << std::endl;
		exit(1);
	}

	EngineProcess();
	return 0;
}
