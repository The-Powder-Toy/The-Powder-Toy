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

VideoBuffer * SaveRenderer::Render(GameSave * save, bool decorations, bool fire)
{
	std::lock_guard<std::mutex> gx(renderMutex);

	int width, height;
	VideoBuffer * tempThumb = NULL;
	width = save->blockWidth;
	height = save->blockHeight;
	bool doCollapse = save->Collapsed();

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
	if(doCollapse)
		save->Collapse();

	return tempThumb;
}

VideoBuffer * SaveRenderer::Render(unsigned char * saveData, int dataSize, bool decorations, bool fire)
{
	std::lock_guard<std::mutex> g(renderMutex);

	GameSave * tempSave;
	try {
		tempSave = new GameSave((char*)saveData, dataSize);
	} catch (std::exception & e) {

		//Todo: make this look a little less shit
		VideoBuffer * buffer = new VideoBuffer(64, 64);
		buffer->BlendCharacter(32, 32, 'x', 255, 255, 255, 255);

		return buffer;
	}
	VideoBuffer * thumb = Render(tempSave, decorations, fire);
	delete tempSave;

	return thumb;
}

SaveRenderer::~SaveRenderer()
{
}

void SaveRenderer::CopyModes(Renderer *source)
{
	ren->SetRenderMode(source->GetRenderMode());
	ren->SetDisplayMode(source->GetDisplayMode());
	ren->SetColourMode(source->GetColourMode());
}

void SaveRenderer::ResetModes()
{
	ren->ResetModes();
}
