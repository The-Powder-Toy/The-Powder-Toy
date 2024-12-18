#pragma once
#include "Brush.h"
#include <cmath>

class EllipseBrush: public Brush
{
	bool perfectCircle;

public:
	EllipseBrush(bool newPerfectCircle) :
		perfectCircle(newPerfectCircle)
	{
	}
	virtual ~EllipseBrush() override = default;

	PlaneAdapter<std::vector<unsigned char>> GenerateBitmap() const override
	{
		ui::Point size = radius * 2 + Vec2{ 1, 1 };
		PlaneAdapter<std::vector<unsigned char>> bitmap(size);

		int rx = radius.X;
		int ry = radius.Y;

		if (!rx)
		{
			for (int j = 0; j <= 2*ry; j++)
			{
				bitmap[{ rx, j }] = 255;
			}
		}
		else
		{
			int yTop = ry+1, yBottom, i;
			for (i = 0; i <= rx; i++)
			{
				if (perfectCircle)
				{
					while (pow(i - rx, 2.0) * pow(ry - 0.5, 2.0) + pow(yTop - ry, 2.0) * pow(rx - 0.5, 2.0) <= pow(rx, 2.0) * pow(ry, 2.0))
						yTop++;
				}
				else
				{
					while (pow(i - rx, 2.0) * pow(ry, 2.0) + pow(yTop - ry, 2.0) * pow(rx, 2.0) <= pow(rx, 2.0) * pow(ry, 2.0))
						yTop++;
				}
				yBottom = 2*ry - yTop;
				for (int j = 0; j <= ry*2; j++)
				{
					if (j > yBottom && j < yTop)
					{
						bitmap[{ i, j }] = 255;
						bitmap[{ 2*rx-i, j }] = 255;
					}
					else
					{
						bitmap[{ i, j }] = 0;
						bitmap[{ 2*rx-i, j }] = 0;
					}
				}
			}
			bitmap[{ size.X/2, 0 }] = 255;
			bitmap[{ size.X/2, size.Y-1 }] = 255;
		}
		return bitmap;
	}

	std::unique_ptr<Brush> Clone() const override
	{
		return std::make_unique<EllipseBrush>(*this);
	}
};
