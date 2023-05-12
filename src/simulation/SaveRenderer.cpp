#include "SaveRenderer.h"

#include "client/GameSave.h"

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "Simulation.h"

SaveRenderer::SaveRenderer(){
	sim = new Simulation();
	ren = new Renderer(sim);
	ren->decorations_enable = true;
	ren->blackDecorations = true;
}

void SaveRenderer::Flush(int begin, int end)
{
	std::lock_guard<std::mutex> gx(renderMutex);
	std::fill(ren->graphicscache + begin, ren->graphicscache + end, gcache_item());
}

std::unique_ptr<VideoBuffer> SaveRenderer::Render(const GameSave *save, bool decorations, bool fire, Renderer *renderModeSource)
{
	std::lock_guard<std::mutex> gx(renderMutex);

	ren->ResetModes();
	if (renderModeSource)
	{
		ren->SetRenderMode(renderModeSource->GetRenderMode());
		ren->SetDisplayMode(renderModeSource->GetDisplayMode());
		ren->SetColourMode(renderModeSource->GetColourMode());
	}

	std::unique_ptr<VideoBuffer> tempThumb;

	sim->clear_sim();

	if(!sim->Load(save, true))
	{
		ren->decorations_enable = true;
		ren->blackDecorations = !decorations;
		ren->ClearAccumulation();
		ren->clearScreen();

		if (fire)
		{
	   		int frame = 15;
			while(frame)
			{
				frame--;
				ren->render_parts();
				ren->render_fire();
				ren->clearScreen();
			}
		}

		ren->RenderBegin();
		ren->RenderEnd();

		tempThumb = std::make_unique<VideoBuffer>(save->blockSize * CELL);
		tempThumb->BlendImage(ren->Data(), 0xFF, ren->Size().OriginRect());
	}

	return tempThumb;
}

SaveRenderer::~SaveRenderer()
{
	delete ren;
	delete sim;
}
