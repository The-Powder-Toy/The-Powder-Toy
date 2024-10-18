#pragma once
#include "Brush.h"

class RectangleBrush: public Brush
{
public:
	virtual ~RectangleBrush() override = default;

	PlaneAdapter<std::vector<unsigned char>> GenerateBitmap() const override
	{
		auto size = GetSize();
		PlaneAdapter<std::vector<unsigned char>> bitmap(size, 0xFF);
		return bitmap;
	}

	std::unique_ptr<Brush> Clone() const override
	{
		return std::make_unique<RectangleBrush>(*this);
	}
};
