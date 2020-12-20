/*
 * BitmapBrush.h
 *
 *  Created on: Nov 18, 2012
 *      Author: Simon Robertshaw
 */

#ifndef BTIMAPBRUSH_H_
#define BTIMAPBRUSH_H_

#include <vector>
#include "Brush.h"

class BitmapBrush: public Brush
{
protected:
	ui::Point origSize;
	unsigned char * origBitmap;
public:
	BitmapBrush(std::vector<unsigned char> newBitmap, ui::Point rectSize);
	void GenerateBitmap() override;
	virtual ~BitmapBrush();
};

#endif /* BTIMAPBRUSH_H_ */
