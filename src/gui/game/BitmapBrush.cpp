#include "BitmapBrush.h"
#include "Misc.h"
#include <algorithm>
#include <cmath>

BitmapBrush::BitmapBrush(ui::Point inputSize, unsigned char const *inputBitmap)
{
	ui::Point newSize = inputSize;

	//Ensure the rect has odd dimensions so we can pull an integer radius with a 1x1 centre
	if (!(newSize.X % 2))
		newSize.X += 1;
	if (!(newSize.Y % 2))
		newSize.Y += 1;

	origSize = newSize;
	origBitmap = PlaneAdapter<std::vector<unsigned char>>(newSize, 0);
	for (int y = 0; y < inputSize.Y; y++)
		for (int x = 0; x < inputSize.X; x++)
			origBitmap[{ x, y }] = inputBitmap[x + y * inputSize.X];
}

BitmapBrush::BitmapBrush(const BitmapBrush &other) : BitmapBrush(other.origSize, other.origBitmap.data())
{
}

PlaneAdapter<std::vector<unsigned char>> BitmapBrush::GenerateBitmap() const
{
	ui::Point size = radius * 2 + Vec2{ 1, 1 };
	PlaneAdapter<std::vector<unsigned char>> bitmap;
	if (size == origSize)
		bitmap = origBitmap;
	else
	{
		bitmap = PlaneAdapter<std::vector<unsigned char>>(size);
		//Bilinear interpolation
		float factorX = ((float)origSize.X)/((float)size.X);
		float factorY = ((float)origSize.Y)/((float)size.Y);
		for (int y = 0; y < size.Y; y++)
		{
			for (int x = 0; x < size.X; x++)
			{
				float originalY = ((float)y)*factorY;
				float originalX = ((float)x)*factorX;

				auto lowerX = int(std::floor(originalX));
				auto upperX = int(std::min((float)(origSize.X-1), std::floor(originalX+1.0f)));
				auto lowerY = int(std::floor(originalY));
				auto upperY = int(std::min((float)(origSize.Y-1), std::floor(originalY+1.0f)));

				unsigned char topRight = origBitmap[{ upperX, lowerY }];
				unsigned char topLeft = origBitmap[{ lowerX, lowerY }];
				unsigned char bottomRight = origBitmap[{ upperX, upperY }];
				unsigned char bottomLeft = origBitmap[{ lowerX, upperY }];
				float top = LinearInterpolate<float>(topLeft, topRight, float(lowerX), float(upperX), originalX);
				float bottom = LinearInterpolate<float>(bottomLeft, bottomRight, float(lowerX), float(upperX), originalX);
				float mid = LinearInterpolate<float>(top, bottom, float(lowerY), float(upperY), originalY);
				bitmap[{ x, y }] = mid > 128 ? 255 : 0;
			}
		}
	}
	return bitmap;
}

std::unique_ptr<Brush> BitmapBrush::Clone() const
{
	return std::make_unique<BitmapBrush>(*this);
}
