/*
 * Brush.h
 *
 *  Created on: Jan 22, 2012
 *      Author: Simon
 */

#ifndef BRUSH_H_
#define BRUSH_H_

#include <iostream>
#include "interface/Point.h"

class Brush
{
protected:
	unsigned char * outline;
	unsigned char * bitmap;
	ui::Point size;
	ui::Point radius;
	void updateOutline()
	{
		if(!bitmap)
			GenerateBitmap();
		if(!bitmap)
			return;
		if(outline)
			free(outline);
		outline = (unsigned char *)calloc(size.X*size.Y, sizeof(unsigned char));
		for(int x = 0; x < size.X; x++)
		{
			for(int y = 0; y < size.Y; y++)
			{
				if(bitmap[y*size.X+x] && (!y || !x || y == size.X-1 || x == size.Y-1 || !bitmap[y*size.X+(x+1)] || !bitmap[y*size.X+(x-1)] || !bitmap[(y-1)*size.X+x] || !bitmap[(y+1)*size.X+x]))
					outline[y*size.X+x] = 255;
				else
					outline[y*size.X+x] = 0;
			}
		}
	}
public:
	Brush(ui::Point size_):
		bitmap(NULL),
		outline(NULL),
		radius(0, 0),
		size(0, 0)
	{
		SetRadius(size_);
	};
	ui::Point GetRadius()
	{
		return radius;
	}
	void SetRadius(ui::Point radius)
	{
		this->radius = radius;
		this->size = radius+radius+ui::Point(1, 1);
		GenerateBitmap();
		updateOutline();
	}
	virtual ~Brush() {
		if(bitmap)
			delete bitmap;
		if(outline)
			delete outline;
	}
	virtual void RenderRect(Graphics * g, ui::Point position1, ui::Point position2)
	{
		int width, height, t;
		width = position2.X-position1.X;
		height = position2.Y-position1.Y;
		if(height<0)
		{
			position1.Y += height;
			height *= -1;
		}
		if(width<0)
		{
			position1.X += width;
			width *= -1;
		}
		g->xor_line(position1.X, position1.Y, position1.X+width, position1.Y);
		g->xor_line(position1.X, position1.Y+height, position1.X+width, position1.Y+height);
		g->xor_line(position1.X+width, position1.Y+1, position1.X+width, position1.Y+height-1);
		g->xor_line(position1.X, position1.Y+1, position1.X, position1.Y+height-1);
	}
	virtual void RenderLine(Graphics * g, ui::Point position1, ui::Point position2)
	{
		g->xor_line(position1.X, position1.Y, position2.X, position2.Y);
	}
	//Draw the brush outline onto the screen
	virtual void RenderPoint(Graphics * g, ui::Point position)
	{
		if(!outline)
			updateOutline();
		if(!outline)
			return;
		g->xor_bitmap(outline, position.X-radius.X, position.Y-radius.Y-1, size.X, size.Y);
	}
	virtual void GenerateBitmap()
	{
		if(bitmap)
			free(bitmap);
		bitmap = (unsigned char *)calloc((size.X*size.Y), sizeof(unsigned char));
		for(int x = 0; x < size.X; x++)
		{
			for(int y = 0; y < size.Y; y++)
			{
				bitmap[(y*size.X)+x] = 255;
			}
		}
	}
	//Get a bitmap for drawing particles
	unsigned char * GetBitmap()
	{
		if(!bitmap)
			GenerateBitmap();
		return bitmap;
	}

	unsigned char * GetOutline()
	{
		if(!outline)
			updateOutline();
		if(!outline)
			return NULL;
		return outline;
	}
};


#endif /* BRUSH_H_ */
