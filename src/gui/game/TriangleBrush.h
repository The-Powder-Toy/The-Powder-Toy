/*
 * TriangleBrush.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Savely Skresanov
 */

#ifndef TRIANGLEBRUSH_H_
#define TRIANGLEBRUSH_H_

#include <cmath>
#include "Brush.h"

class TriangleBrush: public Brush
{
public:
	TriangleBrush(ui::Point size_):
		Brush(size_)
	{
		SetRadius(size_);
	};
	virtual void GenerateBitmap()
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

#endif /* TRIANGLEBRUSH_H_ */
