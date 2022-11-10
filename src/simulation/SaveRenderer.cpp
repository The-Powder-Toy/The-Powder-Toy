#include "SaveRenderer.h"

#include "client/GameSave.h"

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "Simulation.h"

SaveRenderer::SaveRenderer(){
	g = new Graphics();
	sim = new Simulation();
	ren = new Renderer(g, sim);
	ren->decorations_enable = true;
	ren->blackDecorations = true;
}

void SaveRenderer::Flush(int begin, int end)
{
	std::lock_guard<std::mutex> gx(renderMutex);
	std::fill(ren->graphicscache + begin, ren->graphicscache + end, gcache_item());
}

VideoBuffer * SaveRenderer::Render(GameSave * save, bool decorations, bool fire, Renderer *renderModeSource)
{
	std::lock_guard<std::mutex> gx(renderMutex);

	ren->ResetModes();
	if (renderModeSource)
	{
		ren->SetRenderMode(renderModeSource->GetRenderMode());
		ren->SetDisplayMode(renderModeSource->GetDisplayMode());
		ren->SetColourMode(renderModeSource->GetColourMode());
	}

	int width, height;
	VideoBuffer * tempThumb = NULL;
	width = save->blockWidth;
	height = save->blockHeight;

	g->Clear();
	sim->clear_sim();

	if(!sim->Load(save, true))
	{
		ren->decorations_enable = true;
		ren->blackDecorations = !decorations;
		pixel * pData = NULL;
		pixel * dst;
		pixel * src = g->vid;

		ren->ClearAccumulation();

		if (fire)
		{
	   		int frame = 15;
			while(frame)
			{
				frame--;
				ren->render_parts();
				ren->render_fire();
				ren->clearScreen(1.0f);
			}
		}

		ren->RenderBegin();
		ren->RenderEnd();


		pData = (pixel *)malloc(PIXELSIZE * ((width*CELL)*(height*CELL)));
		dst = pData;
		for(int i = 0; i < height*CELL; i++)
		{
			memcpy(dst, src, (width*CELL)*PIXELSIZE);
			dst+=(width*CELL);///PIXELSIZE;
			src+=WINDOWW;
		}
		tempThumb = new VideoBuffer(pData, width*CELL, height*CELL);
		free(pData);
	}

	return tempThumb;
}

SaveRenderer::~SaveRenderer()
{
}
