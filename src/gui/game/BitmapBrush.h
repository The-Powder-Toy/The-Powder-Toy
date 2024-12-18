#pragma once
#include "Brush.h"

class BitmapBrush: public Brush
{
	ui::Point origSize{ 0, 0 };
	// 2D array with coords [0, origSize.X) by [0, origSize.Y)
	PlaneAdapter<std::vector<unsigned char>> origBitmap;

public:
	BitmapBrush(ui::Point size, unsigned char const *bitmap);
	BitmapBrush(const BitmapBrush &other);
	virtual ~BitmapBrush() override = default;
	PlaneAdapter<std::vector<unsigned char>> GenerateBitmap() const override;

	std::unique_ptr<Brush> Clone() const override;
};
