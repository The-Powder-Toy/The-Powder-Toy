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
		Data = new pixel[thumb.Size.X*thumb.Size.Y];
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
		Data = new pixel[_size.X*_size.Y];
		memcpy(Data, _data, (_size.X*_size.Y) * PIXELSIZE);
	}
	else
	{
		Data = NULL;
	}
}

void Thumbnail::Resize(int width, int height)
{
	Resize(ui::Point(width, height));
}

void Thumbnail::Resize(ui::Point newSize)
{
	float scaleFactorX = 1.0f, scaleFactorY = 1.0f;
	if(Size.Y >  newSize.Y)
	{
		scaleFactorY = float(newSize.Y)/((float)Size.Y);
	}
	if(Size.X > newSize.X)
	{
		scaleFactorX = float(newSize.X)/((float)Size.X);
	}
	if(newSize.X == -1)
		scaleFactorX = scaleFactorY;
	if(newSize.Y == -1)
		scaleFactorY = scaleFactorX;
	if(scaleFactorY < 1.0f || scaleFactorX < 1.0f)
	{
		float scaleFactor = scaleFactorY < scaleFactorX ? scaleFactorY : scaleFactorX;
		pixel * thumbData = Data;
		Data = Graphics::resample_img(thumbData, Size.X, Size.Y, Size.X * scaleFactor, Size.Y * scaleFactor);
		Size.X *= scaleFactor;
		Size.Y *= scaleFactor;
		delete[] thumbData;
	}
}

Thumbnail::~Thumbnail()
{
	delete[] Data;
}
