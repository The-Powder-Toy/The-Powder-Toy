/*
 * Thumbnail.cpp
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#include "Thumbnail.h"
#include "simulation/Simulation.h"
#include "simulation/SaveLoader.h"
#include "Renderer.h"

Thumbnail::Thumbnail(const Thumbnail & thumb):
	ID(thumb.ID),
	Datestamp(thumb.Datestamp),
	Data(thumb.Data),
	Size(thumb.Size)
{
	//Ensure the actual thumbnail data is copied
	if(thumb.Data)
	{
		Data = (pixel *)malloc((thumb.Size.X*thumb.Size.Y) * PIXELSIZE);
		memcpy(Data, thumb.Data, (thumb.Size.X*thumb.Size.Y) * PIXELSIZE);
	}
	else
	{
		Data = NULL;
	}
}

Thumbnail::Thumbnail(int _id, int _datestamp, pixel * _data, ui::Point _size):
	ID(_id),
	Datestamp(_datestamp),
	Data(_data),
	Size(_size)
{
}

Thumbnail::Thumbnail(Save * save):
	ID(0),
	Datestamp(0),
	Data(NULL),
	Size(XRES+BARSIZE, YRES+MENUSIZE)
{
	Graphics * g = new Graphics();
	Simulation * sim = new Simulation();
	Renderer * ren = new Renderer(g, sim);
	sim->Load(save->GetData(), save->GetDataLength());
	ren->render_parts();

	int width, height;

	pixel * dst;
	pixel * src = g->vid;

	if(SaveLoader::Info(save->GetData(), save->GetDataLength(), width, height))
		goto fail;

	dst = Data = (pixel *)malloc(PIXELSIZE * ((width*CELL)*(height*CELL)));

	for(int i = 0; i < height*CELL; i++)
	{
		memcpy(dst, src, (width*CELL)*PIXELSIZE);
		dst+=(width*CELL);///PIXELSIZE;
		src+=XRES+BARSIZE;
	}

	Size = ui::Point(width*CELL, height*CELL);
fail:
	delete ren;
	delete sim;
	delete g;
}

Thumbnail::~Thumbnail()
{
	if(Data)
	{
		free(Data);
	}
}
