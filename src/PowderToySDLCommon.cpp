#include "PowderToySDL.h"
#include "gui/interface/Engine.h"

void MainLoop()
{
	while (ui::Engine::Ref().Running())
	{
		auto delay = EngineProcess();
		if (delay.has_value())
		{
			SDL_Delay(std::max(*delay, UINT64_C(1)));
		}
	}
}

void ApplyFpsLimit()
{
}
