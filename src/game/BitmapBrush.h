/*
 * BitmapBrush.h
 *
 *  Created on: Nov 18, 2012
 *      Author: Simon Robertshaw
 */

#ifndef BTIMAPBRUSH_H_
#define BTIMAPBRUSH_H_

#include <vector>
#include <cmath>
#include "Brush.h"

class BitmapBrush: public Brush
{
public:
	BitmapBrush(std::vector<unsigned char> newBitmap, ui::Point rectSize_):
		Brush(ui::Point(0, 0))
	{
		ui::Point newSize = rectSize_;

		//Ensure the rect has odd dimentions so we can pull an integer radius with a 1x1 centre 
		if(!(newSize.X % 2))
			newSize.X += 1;
		if(!(newSize.Y % 2))
			newSize.Y += 1;

		radius = (newSize-ui::Point(1, 1))/2;
		size = newSize;

		if(bitmap)
			delete[] bitmap;
		bitmap = new unsigned char[size.X*size.Y];
		std::fill(bitmap, bitmap+(size.X*size.Y), 0);
		for(int y = 0; y < rectSize_.Y; y++)
		{
			for(int x = 0; x < rectSize_.X; x++)
			{
				bitmap[(y*size.X)+x] = newBitmap[(y*rectSize_.X)+x];
			}	
		}

		updateOutline();
	};
	virtual void SetRadius(ui::Point radius)
	{
		//Do nothing... this brush is a fixed size
	}
	virtual void GenerateBitmap()
	{
		//Do nothing
	}
};

#endif /* BTIMAPBRUSH_H_ */
