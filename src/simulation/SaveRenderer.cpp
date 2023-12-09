#include "SaveRenderer.h"

#include "client/GameSave.h"

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "Simulation.h"
#include "SimulationData.h"

SaveRenderer::SaveRenderer()
{
	sim = std::make_unique<Simulation>();
	ren = std::make_unique<Renderer>(sim.get());
	ren->decorations_enable = true;
	ren->blackDecorations = true;
}

SaveRenderer::~SaveRenderer() = default;

std::unique_ptr<VideoBuffer> SaveRenderer::Render(const GameSave *save, bool decorations, bool fire, Renderer *renderModeSource)
{
	// this function usually runs on a thread different from where element info in SimulationData may be written, so we acquire a read-only lock on it
	auto &sd = SimulationData::CRef();
	std::shared_lock lk(sd.elementGraphicsMx);
	std::lock_guard<std::mutex> gx(renderMutex);

	ren->ResetModes();
	if (renderModeSource)
	{
		ren->SetRenderMode(renderModeSource->GetRenderMode());
		ren->SetDisplayMode(renderModeSource->GetDisplayMode());
		ren->SetColourMode(renderModeSource->GetColourMode());
	}

	sim->clear_sim();

	sim->Load(save, true, { 0, 0 });
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

	auto tempThumb = std::make_unique<VideoBuffer>(save->blockSize * CELL);
	tempThumb->BlendImage(ren->Data(), 0xFF, ren->Size().OriginRect());

	return tempThumb;
}
