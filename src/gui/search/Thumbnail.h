#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <iostream>
#include "graphics/Graphics.h"
#include "gui/interface/Point.h"

class Thumbnail
{
public:
	Thumbnail(const Thumbnail & thumb);

	Thumbnail(int _id, int _datestamp, pixel * _data, ui::Point _size);

	~Thumbnail();

	void Resize(int Width, int Height);
	void Resize(ui::Point newSize);

	int ID, Datestamp;
	pixel * Data;
	ui::Point Size;
};

#endif // THUMBNAIL_H
