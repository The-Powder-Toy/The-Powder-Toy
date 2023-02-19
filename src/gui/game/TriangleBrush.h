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
	ui::Point radius;

public:
	TriangleBrush(ui::Point radius):
		Brush(),
		radius(radius)
	{
	}

	virtual ~TriangleBrush() override = default;

	std::pair<ui::Point, std::unique_ptr<unsigned char []>> GenerateBitmap() const override
	{
		ui::Point size = radius * 2 + 1;
		auto bitmap = std::make_unique<unsigned char []>(size.X * size.Y);

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
		return std::make_pair(radius, std::move(bitmap));
	}

	ui::Point GetRadius() const override
	{
		return radius;
	}

	void SetRadius(ui::Point radius) override
	{
		this->radius = radius;
		InvalidateCache();
	}
};
