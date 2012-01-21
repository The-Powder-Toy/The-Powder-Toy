#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <iostream>
#include "Graphics.h"
#include "interface/Point.h"

class Thumbnail
{
public:
	Thumbnail(const Thumbnail & thumb):
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

	Thumbnail(int _id, int _datestamp, pixel * _data, ui::Point _size):
		ID(_id),
		Datestamp(_datestamp),
		Data(_data),
		Size(_size)
	{
	}

	~Thumbnail()
	{
		if(Data)
		{
			free(Data);
		}
	}

	int ID, Datestamp;
	ui::Point Size;
	pixel * Data;
};

#endif // THUMBNAIL_H
