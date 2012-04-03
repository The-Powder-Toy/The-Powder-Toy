/*
 * Thumbnail.cpp
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#include "Thumbnail.h"

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
	if(_data)
	{
		Data = (pixel *)malloc((_size.X*_size.Y) * PIXELSIZE);
		memcpy(Data, _data, (_size.X*_size.Y) * PIXELSIZE);
	}
	else
	{
		Data = NULL;
	}
}

Thumbnail::~Thumbnail()
{
	if(Data)
	{
		free(Data);
	}
}
