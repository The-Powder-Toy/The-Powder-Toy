#pragma once
#include "Brush.h"
#include <cmath>

class TriangleBrush: public Brush
{
public:
	virtual ~TriangleBrush() override = default;

	PlaneAdapter<std::vector<unsigned char>> GenerateBitmap() const override
	{
		ui::Point size = radius * 2 + Vec2{ 1, 1 };
		PlaneAdapter<std::vector<unsigned char>> bitmap(size);

		int rx = radius.X;
		int ry = radius.Y;
		for(int x = -rx; x <= rx; x++)
		{
			for(int y = -ry; y <= ry; y++)
			{
				if ((abs((rx+2*x)*ry+rx*y) + abs(2*rx*(y-ry)) + abs((rx-2*x)*ry+rx*y))<=(4*rx*ry))
				{
					bitmap[{ x+rx, y+ry }] = 255;
				}
				else
				{
					bitmap[{ x+rx, y+ry }] = 0;
				}
			}
		}
		return bitmap;
	}

	std::unique_ptr<Brush> Clone() const override
	{
		return std::make_unique<TriangleBrush>(*this);
	}
};
