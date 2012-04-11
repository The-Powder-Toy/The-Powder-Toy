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
	bool * outline;
	bool * bitmap;
	ui::Point size;
	void updateOutline()
	{
		if(!bitmap)
			GenerateBitmap();
		if(!bitmap)
			return;
		if(outline)
			free(outline);
		int width = size.X*2;
		int height = size.Y*2;
		outline = (bool *)malloc(sizeof(bool)*((width+1)*(height+1)));
		for(int x = 0; x <= width; x++)
		{
			for(int y = 0; y <= height; y++)
			{
				if(bitmap[y*width+x] && (!y || !x || y == height || x == width || !bitmap[y*width+(x+1)] || !bitmap[y*width+(x-1)] || !bitmap[(y-1)*width+x] || !bitmap[(y+1)*width+x]))
					outline[y*width+x] = true;
				else
					outline[y*width+x] = false;
			}
		}
	}
public:
	Brush(ui::Point size_):
		bitmap(NULL),
		size(size_),
		outline(NULL)
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
		for(int x = 0; x <= size.X*2; x++)
		{
			for(int y = 0; y <= size.Y*2; y++)
			{
				if(outline[y*(size.X*2)+x])
					g->xor_pixel(position.X-size.X+x, position.Y-size.Y+y);
			}
		}
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

	bool * GetOutline()
	{
		if(!outline)
			updateOutline();
		if(!outline)
			return NULL;
		return outline;
	}
};


#endif /* BRUSH_H_ */
