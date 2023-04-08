#include "PowderToySDL.h"
#include "graphics/Graphics.h"
#include "common/platform/Platform.h"
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
	std::unique_ptr<ui::Engine> engine;
};
static std::unique_ptr<ExplicitSingletons> explicitSingletons;

int main(int argc, char * argv[])
{
	Platform::SetupCrt();
	Platform::Atexit([]() {
		SDLClose();
		explicitSingletons.reset();
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
	engine.Begin();
	engine.SetFastQuit(true);

	if (argc >= 2)
	{
		engine.ShowWindow(new FontEditor(argv[1]));
	}
	else
	{
		std::cerr << "path to font.cpp not supplied" << std::endl;
		Platform::Exit(1);
	}

	EngineProcess();
	Platform::Exit(0);
	return 0;
}
