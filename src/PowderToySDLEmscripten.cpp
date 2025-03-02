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

void ApplyFpsLimit()
{
	static bool mainLoopSet = false;
	if (!mainLoopSet)
	{
		emscripten_set_main_loop(MainLoopBody, 0, 0);
		mainLoopSet = true;
	}
	// this generally attempts to replicate the behaviour of EngineProcess
	std::optional<float> drawLimit;
	auto &engine = ui::Engine::Ref();
	auto fpsLimit = engine.GetFpsLimit();
	if (auto *fpsLimitExplicit = std::get_if<FpsLimitExplicit>(&fpsLimit))
	{
		drawLimit = fpsLimitExplicit->value;
	}
	else if (std::holds_alternative<FpsLimitFollowDraw>(fpsLimit))
	{
		auto effectiveDrawLimit = engine.GetEffectiveDrawCap();
		if (effectiveDrawLimit)
		{
			drawLimit = float(*effectiveDrawLimit);
		}
		// else // TODO: DrawLimitVsync
		// {
		// 	if (std::holds_alternative<DrawLimitVsync>(engine.GetDrawingFrequencyLimit()))
		// 	{
		// 		emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
		// 		std::cerr << "implicit fps limit via vsync" << std::endl;
		// 		return;
		// 	}
		// }
	}
	int delay = 0; // no cap
	if (drawLimit.has_value())
	{
		delay = int(1000.f / *drawLimit);
	}
	emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, delay);
	std::cerr << "explicit fps limit: " << delay << "ms delays" << std::endl;
}

// Is actually only called once at startup, the real main loop body is MainLoopBody.
void MainLoop()
{
	ApplyFpsLimit();
	MainLoopBody();
}
