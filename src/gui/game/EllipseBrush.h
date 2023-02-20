#pragma once
#include "common/RasterGeometry.h"
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

	std::unique_ptr<unsigned char []> GenerateBitmap() const override
	{
		ui::Point size = radius * 2 + ui::Point(1, 1);
		auto bitmap = std::make_unique<unsigned char []>(size.X * size.Y);

		std::fill(&bitmap[0], &bitmap[size.X * size.Y], 0);
		float rx = radius.X, ry = radius.Y;
		if (perfectCircle)
		{
			rx += 0.5;
			ry += 0.5;
		}
		RasterizeEllipseRows(Vec2<float>(rx * rx, ry * ry), [this, &bitmap, size](int xLim, int y)
			{
				for (int x = -xLim; x <= xLim; x++)
					bitmap[x + radius.X + (y + radius.Y) * size.X] = 0xFF;
			});
		return bitmap;
	}

	std::unique_ptr<Brush> Clone() const override
	{
		return std::make_unique<EllipseBrush>(*this);
	}
};
