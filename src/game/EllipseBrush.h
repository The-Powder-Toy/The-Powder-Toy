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
		bitmap = (bool *)malloc(sizeof(bool)*(((size.X*2)+1)*((size.Y*2)+1)));
		int rx = size.X;
		int ry = size.Y;
		for(int x = 0; x <= size.X*2; x++)
		{
			for(int y = 0; y <= size.Y*2; y++)
			{
				if((pow(x-size.X,2)*pow(ry,2)+pow(y-size.Y,2)*pow(rx,2)<=pow(rx,2)*pow(ry,2)))
				{
					bitmap[y*(size.X*2)+x] = true;
				}
				else
				{
					bitmap[y*(size.X*2)+x] = false;
				}
			}
		}
	}
};

#endif /* ELIPSEBRUSH_H_ */
