#include "Brush.h"
#include "graphics/Graphics.h"

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
	if (newRadius.X < 0)
		newRadius.X = 0;
	if (newRadius.Y < 0)
		newRadius.Y = 0;
	if (newRadius.X > 200)
		newRadius.X = 200;
	if (newRadius.Y > 200)
		newRadius.Y = 200;
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

	if (keepY)
		SetRadius(ui::Point(newSize.X, oldSize.Y));
	else if (keepX)
		SetRadius(ui::Point(oldSize.X, newSize.Y));
	else
		SetRadius(newSize);
}

void Brush::RenderRect(Graphics *g, ui::Point position1, ui::Point position2) const
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

	g->XorLine(position1, position1 + Vec2{ width, 0 });
	if (height > 0)
	{
		g->XorLine(position1 + Vec2{ 0, height }, position1 + Vec2{ width, height });
		if (height > 1)
		{
			g->XorLine(position1 + Vec2{ width, 1 }, position1 + Vec2{ width, height - 1 });
			if (width > 0)
			{
				g->XorLine(position1 + Vec2{ 0, 1 }, position1 + Vec2{ 0, height - 1 });
			}
		}
	}
}

void Brush::RenderLine(Graphics *g, ui::Point position1, ui::Point position2) const
{
	g->XorLine(position1, position2);
}

void Brush::RenderPoint(Graphics *g, ui::Point position) const
{
	g->XorImage(&outline[0], RectBetween(position - radius, position + radius));
}

void Brush::RenderFill(Graphics *g, ui::Point position) const
{
	g->XorLine(position - Vec2{ 5, 0 }, position - Vec2{ 1, 0 });
	g->XorLine(position + Vec2{ 5, 0 }, position + Vec2{ 1, 0 });
	g->XorLine(position - Vec2{ 0, 5 }, position - Vec2{ 0, 1 });
	g->XorLine(position + Vec2{ 0, 5 }, position + Vec2{ 0, 1 });
}
