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

std::pair<std::unique_ptr<VideoBuffer>, std::vector<ByteString>> SaveRenderer::Render(const GameSave *save, bool decorations, bool fire, Renderer *renderModeSource)
{
	std::lock_guard<std::mutex> gx(renderMutex);

	ren->ResetModes();
	if (renderModeSource)
	{
		ren->SetRenderMode(renderModeSource->GetRenderMode());
		ren->SetDisplayMode(renderModeSource->GetDisplayMode());
		ren->SetColourMode(renderModeSource->GetColourMode());
	}

	sim->clear_sim();

	auto missingElementTypes = sim->Load(save, true, { 0, 0 });
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

	return { std::move(tempThumb), missingElementTypes };
}

SaveRenderer::~SaveRenderer()
{
	delete ren;
	delete sim;
}
