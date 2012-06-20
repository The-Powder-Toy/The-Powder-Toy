/*
 * ElipseBrush.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#ifndef ELIPSEBRUSH_H_
#define ELIPSEBRUSH_H_

#include <cmath>
#include "Brush.h"

class EllipseBrush: public Brush
{
public:
	EllipseBrush(ui::Point size_):
		Brush(size_)
	{
		SetRadius(size_);
	};
	virtual void GenerateBitmap()
	{
		if(bitmap)
			delete[] bitmap;
		bitmap = new unsigned char[size.X*size.Y];
		int rx = radius.X;
		int ry = radius.Y;
		for(int x = 0; x <= radius.X*2; x++)
		{
			for(int y = 0; y <= radius.Y*2; y++)
			{
				if((pow(x-radius.X,2.0f)*pow(ry,2.0f)+pow(y-radius.Y,2.0f)*pow(rx,2.0f)<=pow(rx,2.0f)*pow(ry,2.0f)))
				{
					bitmap[y*(size.X)+x] = 255;
				}
				else
				{
					bitmap[y*(size.X)+x] = 0;
				}
			}
		}
	}
};

#endif /* ELIPSEBRUSH_H_ */
