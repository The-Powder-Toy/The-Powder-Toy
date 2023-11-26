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
	
	WindowFrameOps windowFrameOps;
	if (argc >= 3)
	{
		std::istringstream ss(argv[2]);
		int buf;
		if (ss >> buf)
		{
			windowFrameOps.scale = buf;
		}
	}

	// TODO: maybe bind the maximum allowed scale to screen size somehow
	if (windowFrameOps.scale < 1 || windowFrameOps.scale > 10)
	{
		windowFrameOps.scale = 1;
	}

	explicitSingletons->engine = std::make_unique<ui::Engine>();

	auto &engine = ui::Engine::Ref();
	engine.g = new Graphics();
	engine.windowFrameOps = windowFrameOps;

	SDLOpen();

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

	while (engine.Running())
	{
		EngineProcess();
	}
	Platform::Exit(0);
	return 0;
}
