#include "Brush.h"
#include "graphics/Renderer.h"

Brush::Brush(const Brush &other)
{
	radius = other.radius;
	auto size = GetSize();
	if (other.bitmap)
	{
		bitmap = std::make_unique<unsigned char []>(size.X * size.Y);
		std::copy(&other.bitmap[0], &other.bitmap[0] + size.X * size.Y, &bitmap[0]);
	}
	if (other.outline)
	{
		outline = std::make_unique<unsigned char []>(size.X * size.Y);
		std::copy(&other.outline[0], &other.outline[0] + size.X * size.Y, &outline[0]);
	}
}

void Brush::InitBitmap()
{
	bitmap = GenerateBitmap();
}

void Brush::InitOutline()
{
	InitBitmap();
	ui::Point bounds = GetSize();
	outline = std::make_unique<unsigned char []>(bounds.X * bounds.Y);
	for (int j = 0; j < bounds.Y; j++)
	{
		for (int i = 0; i < bounds.X; i++)
		{
			bool value = false;
			if (bitmap[i + j * bounds.X])
			{
				if (i == 0 || j == 0 || i == bounds.X - 1 || j == bounds.Y - 1)
					value = true;
				else if (!bitmap[(i + 1) + j * bounds.X])
					value = true;
				else if (!bitmap[(i - 1) + j * bounds.X])
					value = true;
				else if (!bitmap[i + (j + 1) * bounds.X])
					value = true;
				else if (!bitmap[i + (j - 1) * bounds.X])
					value = true;
			}
			outline[i + j * bounds.X] = value ? 0xFF : 0;
		}
	}
}

void Brush::SetRadius(ui::Point newRadius)
{
	radius = newRadius;
	InitOutline();
}

void Brush::AdjustSize(int delta, bool logarithmic, bool keepX, bool keepY)
{
	if (keepX && keepY)
		return;

	ui::Point newSize(0, 0);
	ui::Point oldSize = GetRadius();
	if (logarithmic)
		newSize = oldSize + ui::Point(delta * std::max(oldSize.X / 5, 1), delta * std::max(oldSize.Y / 5, 1));
	else
		newSize = oldSize + ui::Point(delta, delta);
	if (newSize.X < 0)
		newSize.X = 0;
	if (newSize.Y < 0)
		newSize.Y = 0;
	if (newSize.X > 200)
		newSize.X = 200;
	if (newSize.Y > 200)
		newSize.Y = 200;

	if (keepY)
		SetRadius(ui::Point(newSize.X, oldSize.Y));
	else if (keepX)
		SetRadius(ui::Point(oldSize.X, newSize.Y));
	else
		SetRadius(newSize);
}

void Brush::RenderRect(Renderer * ren, ui::Point position1, ui::Point position2) const
{
	int width, height;
	width = position2.X-position1.X;
	height = position2.Y-position1.Y;
	if (height<0)
	{
		position1.Y += height;
		height *= -1;
	}
	if (width<0)
	{
		position1.X += width;
		width *= -1;
	}

	ren->xor_line(position1.X, position1.Y, position1.X+width, position1.Y);
	if (height>0){
		ren->xor_line(position1.X, position1.Y+height, position1.X+width, position1.Y+height);
		if (height>1){
			ren->xor_line(position1.X+width, position1.Y+1, position1.X+width, position1.Y+height-1);
			if (width>0)
				ren->xor_line(position1.X, position1.Y+1, position1.X, position1.Y+height-1);
		}
	}
}

void Brush::RenderLine(Renderer * ren, ui::Point position1, ui::Point position2) const
{
	ren->xor_line(position1.X, position1.Y, position2.X, position2.Y);
}

void Brush::RenderPoint(Renderer * ren, ui::Point position) const
{
	ren->xor_bitmap(&outline[0], position.X - radius.X, position.Y - radius.Y, 2 * radius.X + 1, 2 * radius.Y + 1);
}

void Brush::RenderFill(Renderer * ren, ui::Point position) const
{
	ren->xor_line(position.X-5, position.Y, position.X-1, position.Y);
	ren->xor_line(position.X+5, position.Y, position.X+1, position.Y);
	ren->xor_line(position.X, position.Y-5, position.X, position.Y-1);
	ren->xor_line(position.X, position.Y+5, position.X, position.Y+1);
}
