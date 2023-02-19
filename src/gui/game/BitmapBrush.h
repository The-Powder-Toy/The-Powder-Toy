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
	ui::Point radius;
	ui::Point origSize;
	// 2D array with coords [0, origSize.X) by [0, origSize.Y)
	std::unique_ptr<unsigned char []> origBitmap;

public:
	BitmapBrush(ui::Point size, unsigned char const *bitmap);
	virtual ~BitmapBrush() override = default;
	std::pair<ui::Point, std::unique_ptr<unsigned char []>> GenerateBitmap() const override;

	ui::Point GetRadius() const override
	{
		return radius;
	}

	void SetRadius(ui::Point radius) override
	{
		this->radius = radius;
		InvalidateCache();
	}

	std::unique_ptr<Brush> Clone() const override;
};
