#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <iostream>
#include "graphics/Graphics.h"
#include "interface/Point.h"

class Thumbnail
{
public:
	Thumbnail(const Thumbnail & thumb);

	Thumbnail(int _id, int _datestamp, pixel * _data, ui::Point _size);

	~Thumbnail();

	int ID, Datestamp;
	ui::Point Size;
	pixel * Data;
};

#endif // THUMBNAIL_H
