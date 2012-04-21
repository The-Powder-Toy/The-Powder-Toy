/*
 * ElipseBrush.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#ifndef ELIPSEBRUSH_H_
#define ELIPSEBRUSH_H_

#include "Brush.h"

class EllipseBrush: public Brush
{
public:
	EllipseBrush(ui::Point size_):
		Brush(size_)
	{

	};
	virtual void GenerateBitmap()
	{
		if(bitmap)
			free(bitmap);
		bitmap = (unsigned char*)calloc((size.X*size.Y), sizeof(unsigned char));
		int rx = radius.X;
		int ry = radius.Y;
		for(int x = 0; x <= radius.X*2; x++)
		{
			for(int y = 0; y <= radius.Y*2; y++)
			{
				if((pow(x-radius.X,2)*pow(ry,2)+pow(y-radius.Y,2)*pow(rx,2)<=pow(rx,2)*pow(ry,2)))
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
