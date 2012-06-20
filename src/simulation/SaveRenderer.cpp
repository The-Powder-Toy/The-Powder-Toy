/*
 * SaveRenderer.cpp
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#include "SaveRenderer.h"
#include "Graphics.h"
#include "Simulation.h"
#include "Renderer.h"


SaveRenderer::SaveRenderer(){
	g = new Graphics();
	sim = new Simulation();
	ren = new Renderer(g, sim);
}

Thumbnail * SaveRenderer::Render(GameSave * save)
{
#ifndef OGLR
	Thumbnail * tempThumb = NULL;
	int width, height;
	width = save->blockWidth;
	height = save->blockHeight;
	
	pixel * pData = NULL;
	pixel * dst;
	pixel * src = g->vid;
	
	g->Clear();
	sim->clear_sim();
	if(!sim->Load(save))
	{
		ren->render_parts();
	
		pData = (pixel *)malloc(PIXELSIZE * ((width*CELL)*(height*CELL)));
		dst = pData;
		for(int i = 0; i < height*CELL; i++)
		{
			printf("%d\n",i);
			memcpy(dst, src, (width*CELL)*PIXELSIZE);
			dst+=(width*CELL);///PIXELSIZE;
			src+=XRES+BARSIZE;
		}
	
		tempThumb = new Thumbnail(0, 0, pData, ui::Point(width*CELL, height*CELL));
	}
	if(pData)
		free(pData);
	return tempThumb;
#else
	VideoBuffer buffer(64, 64);
	buffer.SetCharacter(32, 32, 'x', 255, 255, 255, 255);
	Thumbnail * thumb = new Thumbnail(0, 0, buffer.Buffer, ui::Point(64, 64));
	return thumb;
#endif
}

Thumbnail * SaveRenderer::Render(unsigned char * saveData, int dataSize)
{
	GameSave * tempSave;
	try {
		tempSave = new GameSave((char*)saveData, dataSize);
	} catch (exception & e) {
		
		//Todo: make this look a little less shit
		VideoBuffer buffer(64, 64);
		buffer.SetCharacter(32, 32, 'x', 255, 255, 255, 255);
		
		Thumbnail * thumb = new Thumbnail(0, 0, buffer.Buffer, ui::Point(64, 64));
		
		return thumb;
	}
	Thumbnail * thumb = Render(tempSave);
	delete tempSave;
	return thumb;
}

SaveRenderer::~SaveRenderer() {
	// TODO Auto-generated destructor stub
}

