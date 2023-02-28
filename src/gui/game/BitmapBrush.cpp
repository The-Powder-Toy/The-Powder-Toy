#include "BitmapBrush.h"
#include "common/tpt-minmax.h"
#include "Misc.h"
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
	origBitmap = std::make_unique<unsigned char []>(newSize.X * newSize.Y);
	std::fill(&origBitmap[0], &origBitmap[newSize.X * newSize.Y], 0);
	for (int y = 0; y < inputSize.Y; y++)
		for (int x = 0; x < inputSize.X; x++)
			origBitmap[x + y * newSize.X] = inputBitmap[x + y * inputSize.X];
}

BitmapBrush::BitmapBrush(const BitmapBrush &other) : BitmapBrush(other.origSize, &other.origBitmap[0])
{
}

std::unique_ptr<unsigned char []> BitmapBrush::GenerateBitmap() const
{
	ui::Point size = radius * 2 + 1;
	auto bitmap = std::make_unique<unsigned char []>(size.X * size.Y);
	if (size == origSize)
		std::copy(&origBitmap[0], &origBitmap[origSize.X * origSize.Y], &bitmap[0]);
	else
	{
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

				unsigned char topRight = origBitmap[(lowerY*origSize.X)+upperX];
				unsigned char topLeft = origBitmap[(lowerY*origSize.X)+lowerX];
				unsigned char bottomRight = origBitmap[(upperY*origSize.X)+upperX];
				unsigned char bottomLeft = origBitmap[(upperY*origSize.X)+lowerX];
				float top = LinearInterpolate<float>(topLeft, topRight, float(lowerX), float(upperX), originalX);
				float bottom = LinearInterpolate<float>(bottomLeft, bottomRight, float(lowerX), float(upperX), originalX);
				float mid = LinearInterpolate<float>(top, bottom, float(lowerY), float(upperY), originalY);
				bitmap[(y*size.X)+x] = mid > 128 ? 255 : 0;
			}
		}
	}
	return bitmap;
}

std::unique_ptr<Brush> BitmapBrush::Clone() const
{
	return std::make_unique<BitmapBrush>(*this);
}
