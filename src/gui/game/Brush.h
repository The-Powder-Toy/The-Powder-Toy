#ifndef BRUSH_H_
#define BRUSH_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include "gui/interface/Point.h"

class Renderer;
class Brush
{
protected:
	std::vector<unsigned char> outline;
	std::vector<unsigned char> bitmap;
	ui::Point size;
	ui::Point radius;
	void updateOutline()
	{
		if (!bitmap.size())
			GenerateBitmap();
		if (!bitmap.size())
			return;

		outline = std::vector<unsigned char>(size.X*size.Y);
		for (int x = 0; x < size.X; x++)
		{
			for (int y = 0; y < size.Y; y++)
			{
				if (bitmap[y*size.X + x] && (!y || !x || x == size.X - 1 || y == size.Y - 1 || !bitmap[y*size.X + (x + 1)] || !bitmap[y*size.X + (x - 1)] || !bitmap[(y - 1)*size.X + x] || !bitmap[(y + 1)*size.X + x]))
				{
					outline[y*size.X + x] = 255;
				}
				else
					outline[y*size.X + x] = 0;
			}
		}
	}
public:
	Brush(ui::Point size_) :
		outline(NULL),
		bitmap(NULL),
		size(0, 0),
		radius(0, 0)
	{
		SetRadius(size_);
	};

	//Radius of the brush 0x0 - infxinf (Radius of 0x0 would be 1x1, radius of 1x1 would be 3x3)
	ui::Point GetRadius()
	{
		return radius;
	}

	//Size of the brush bitmap mask, 1x1 - infxinf
	ui::Point GetSize()
	{
		return size;
	}
	virtual void SetRadius(ui::Point radius)
	{
		this->radius = radius;
		this->size = radius + radius + ui::Point(1, 1);

		GenerateBitmap();
		updateOutline();
	}
	virtual ~Brush() = default;
	virtual void RenderRect(Renderer * ren, ui::Point position1, ui::Point position2);
	virtual void RenderLine(Renderer * ren, ui::Point position1, ui::Point position2);
	virtual void RenderPoint(Renderer * ren, ui::Point position);
	virtual void RenderFill(Renderer * ren, ui::Point position);
	virtual void GenerateBitmap()
	{
		bitmap = std::vector<unsigned char>(size.X * size.Y);
		std::fill_n(std::back_inserter<std::vector<unsigned char>>(bitmap), size.X * size.Y, 255);
	}
	//Get a bitmap for drawing particles
	std::vector<unsigned char> GetBitmap()
	{
		if (!bitmap.size())
			GenerateBitmap();
		return bitmap;
	}

	std::vector<unsigned char> GetOutline()
	{
		if (!outline.size())
			updateOutline();
		if (!outline.size())
		{
			std::vector<unsigned char> uninit;
			return uninit;
		}
		return outline;
	}
};


#endif /* BRUSH_H_ */
