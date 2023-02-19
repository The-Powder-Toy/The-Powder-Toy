#pragma once
#include "Brush.h"

class RectangleBrush: public Brush
{
	ui::Point radius;

public:
	RectangleBrush(ui::Point radius):
		Brush(),
		radius(radius)
	{
	}

	virtual ~RectangleBrush() override = default;

	std::pair<ui::Point, std::unique_ptr<unsigned char []>> GenerateBitmap() const override
	{
		ui::Point size = radius * 2 + 1;
		auto bitmap = std::make_unique<unsigned char []>(size.X * size.Y);
		std::fill(&bitmap[0], &bitmap[size.X * size.Y], 0xFF);
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

	std::unique_ptr<Brush> Clone() const override
	{
		auto into = std::make_unique<RectangleBrush>(radius);
		copyBitmaps(*into);
		return into;
	}
};
