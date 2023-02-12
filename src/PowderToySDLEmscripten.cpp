#include "PowderToySDL.h"
#include "gui/interface/Engine.h"
#include <emscripten.h>
#include <iostream>

static float lastFpsLimit;
static void updateFpsLimit()
{
	lastFpsLimit = ui::Engine::Ref().FpsLimit;
	if (lastFpsLimit == 60.0f) // TODO: rework FPS cap so 60.0 is not the default
	{
		emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
		std::cerr << "implicit fps limit via vsync" << std::endl;
	}
	else
	{
		auto delay = int(1000.f / lastFpsLimit);
		emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, delay);
		std::cerr << "explicit fps limit: " << delay << "ms delays" << std::endl;
	}
}

static void mainLoopBody()
{
	EngineProcess();
	if (lastFpsLimit != ui::Engine::Ref().FpsLimit)
	{
		updateFpsLimit();
	}
}

void MainLoop()
{
	emscripten_set_main_loop(mainLoopBody, 0, 0);
	updateFpsLimit();
	mainLoopBody();
}
