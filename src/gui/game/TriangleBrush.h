/*
 * TriangleBrush.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Savely Skresanov
 */

#pragma once
#include "Brush.h"
#include <cmath>

class TriangleBrush: public Brush
{
public:
	TriangleBrush(ui::Point size_):
		Brush(size_)
	{
		SetRadius(size_);
	};
	void GenerateBitmap() override
	{
		delete[] bitmap;
		bitmap = new unsigned char[size.X*size.Y];
		int rx = radius.X;
		int ry = radius.Y;
		for(int x = -rx; x <= rx; x++)
		{
			for(int y = -ry; y <= ry; y++)
			{
				if ((abs((rx+2*x)*ry+rx*y) + abs(2*rx*(y-ry)) + abs((rx-2*x)*ry+rx*y))<=(4*rx*ry))
				{
					bitmap[(y+ry)*(size.X)+x+rx] = 255;
				}
				else
				{
					bitmap[(y+ry)*(size.X)+x+rx] = 0;
				}
			}
		}
	}
};
