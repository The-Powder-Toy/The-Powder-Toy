#include "PowderToySDL.h"
#include "gui/interface/Engine.h"
#include <emscripten.h>
#include <iostream>

namespace Platform
{
	void MaybeTriggerSyncFs();
}

static void MainLoopBody()
{
	EngineProcess();
	Platform::MaybeTriggerSyncFs();
}

void SetFpsLimit(FpsLimit newFpsLimit)
{
	static bool mainLoopSet = false;
	if (!mainLoopSet)
	{
		emscripten_set_main_loop(MainLoopBody, 0, 0);
		mainLoopSet = true;
	}
	if (auto *fpsLimitVsync = std::get_if<FpsLimitVsync>(&newFpsLimit))
	{
		emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
		std::cerr << "implicit fps limit via vsync" << std::endl;
	}
	else
	{
		auto delay = 0;
		if (auto *fpsLimitExplicit = std::get_if<FpsLimitExplicit>(&newFpsLimit))
		{
			delay = int(1000.f / fpsLimitExplicit->value);
		}
		emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, delay);
		std::cerr << "explicit fps limit: " << delay << "ms delays" << std::endl;
	}
}

void UpdateFpsLimit()
{
	SetFpsLimit(ui::Engine::Ref().GetFpsLimit());
}

// Is actually only called once at startup, the real main loop body is MainLoopBody.
void MainLoop()
{
	UpdateFpsLimit();
	MainLoopBody();
}
