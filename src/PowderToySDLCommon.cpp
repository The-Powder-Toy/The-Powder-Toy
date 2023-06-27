#include "PowderToySDL.h"
#include "gui/interface/Engine.h"

void MainLoop()
{
	while (ui::Engine::Ref().Running())
	{
		EngineProcess();
	}
}

void SetFpsLimit(FpsLimit newFpsLimit)
{
}

void UpdateFpsLimit()
{
}
