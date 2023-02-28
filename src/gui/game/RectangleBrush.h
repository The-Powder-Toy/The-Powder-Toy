#pragma once
#include "Brush.h"

class RectangleBrush: public Brush
{
public:
	virtual ~RectangleBrush() override = default;

	std::unique_ptr<unsigned char []> GenerateBitmap() const override
	{
		auto size = GetSize();
		auto bitmap = std::make_unique<unsigned char []>(size.X * size.Y);
		std::fill(&bitmap[0], &bitmap[0] + size.X * size.Y, 0xFF);
		return bitmap;
	}

	std::unique_ptr<Brush> Clone() const override
	{
		return std::make_unique<RectangleBrush>(*this);
	}
};
