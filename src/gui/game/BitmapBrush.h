/*
 * BitmapBrush.h
 *
 *  Created on: Nov 18, 2012
 *      Author: Simon Robertshaw
 */

#pragma once
#include <vector>
#include "Brush.h"

class BitmapBrush: public Brush
{
protected:
	ui::Point origSize;
	unsigned char * origBitmap;
public:
	BitmapBrush(unsigned char *newBitmap, ui::Point rectSize);
	void GenerateBitmap() override;
	virtual ~BitmapBrush();
};
