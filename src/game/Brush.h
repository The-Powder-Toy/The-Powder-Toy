/*
 * Brush.h
 *
 *  Created on: Jan 22, 2012
 *      Author: Simon
 */

#ifndef BRUSH_H_
#define BRUSH_H_

#include "interface/Point.h"

class Brush
{
protected:
	bool * bitmap;
	ui::Point size;
public:
	Brush(ui::Point size_):
		bitmap(NULL),
		size(size_)
	{

	};
	ui::Point GetRadius()
	{
		return size;
	}
	void SetRadius(ui::Point size)
	{
		this->size = size;
		GenerateBitmap();
	}
	virtual ~Brush() {
		if(bitmap)
			delete bitmap;
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
		g->fillrect(position1.X-1, position1.Y-1, width+2, height+2, 255, 0, 255, 70);
	}
	virtual void RenderLine(Graphics * g, ui::Point position1, ui::Point position2)
	{
		g->blend_line(position1.X, position1.Y, position2.X, position2.Y, 255, 0, 255, 70);
	}
	//Draw the brush outline onto the screen
	virtual void RenderPoint(Graphics * g, ui::Point position)
	{
		g->fillrect(position.X-size.X-1, position.Y-size.Y-1, (size.X*2)+2, (size.Y*2)+2, 255, 0, 255, 70);
	}
	virtual void GenerateBitmap()
	{
		if(bitmap)
			free(bitmap);
		bitmap = (bool *)malloc(sizeof(bool)*(((size.X*2)+1)*((size.Y*2)+1)));
		for(int x = 0; x <= size.X*2; x++)
		{
			for(int y = 0; y <= size.Y*2; y++)
			{
				bitmap[y*(size.X*2)+x] = true;
			}
		}
	}
	//Get a bitmap for drawing particles
	bool * GetBitmap()
	{
		if(!bitmap)
			GenerateBitmap();
		return bitmap;
	}
};


#endif /* BRUSH_H_ */
