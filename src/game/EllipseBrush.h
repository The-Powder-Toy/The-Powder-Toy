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
	//Draw the brush outline onto the screen
	virtual void Render(Graphics * g, ui::Point position)
	{
		if(!bitmap)
			GenerateBitmap();
		//g->fillrect(position.X-size.X-1, position.Y-size.Y-1, (size.X*2)+2, (size.Y*2)+2, 255, 0, 255, 70);
		for(int x = 0; x <= size.X*2; x++)
		{
			for(int y = 0; y <= size.Y*2; y++)
			{
				if(bitmap[y*(size.X*2)+x])
					g->blendpixel(position.X-size.X+x, position.Y-size.Y+y, 255, 0, 255, 70);
			}
		}
	}
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
