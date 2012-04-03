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
#include "SaveLoader.h"


SaveRenderer::SaveRenderer(){
	g = new Graphics();
	sim = new Simulation();
	ren = new Renderer(g, sim);
}

Thumbnail * SaveRenderer::Render(unsigned char * data, int dataLength)
{
	Thumbnail * tempThumb = NULL;
	int width, height;
	pixel * pData = NULL;
	pixel * dst;
	pixel * src = g->vid;

	g->Clear();
	sim->clear_sim();
	if(sim->Load(data, dataLength))
		goto finish;

	if(SaveLoader::Info(data, dataLength, width, height))
		goto finish;

	ren->render_parts();

	dst = pData = (pixel *)malloc(PIXELSIZE * ((width*CELL)*(height*CELL)));

	for(int i = 0; i < height*CELL; i++)
	{
		memcpy(dst, src, (width*CELL)*PIXELSIZE);
		dst+=(width*CELL);///PIXELSIZE;
		src+=XRES+BARSIZE;
	}

	tempThumb = new Thumbnail(0, 0, pData, ui::Point(width*CELL, height*CELL));

finish:
	if(pData)
		free(pData);
	return tempThumb;
}

SaveRenderer::~SaveRenderer() {
	// TODO Auto-generated destructor stub
}

