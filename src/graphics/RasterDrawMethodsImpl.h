#include <cmath>
#include <cstring>
#include "common/RasterGeometry.h"
#include "FontReader.h"
#include "Graphics.h"
#include "RasterDrawMethods.h"

#define clipRect() (static_cast<Derived const &>(*this).GetClipRect())

template<typename Derived, typename V>
static inline void drawPixelUnchecked(RasterDrawMethods<Derived> &self, V Derived::*video, Vec2<int> pos, RGB<uint8_t> colour)
{
	(static_cast<Derived &>(self).*video)[pos] = colour.Pack();
}

template<typename Derived, typename V>
static inline void blendPixelUnchecked(RasterDrawMethods<Derived> &self, V Derived::*video, Vec2<int> pos, RGBA<uint8_t> colour)
{
	pixel &px = (static_cast<Derived &>(self).*video)[pos];
	px = RGB<uint8_t>::Unpack(px).Blend(colour).Pack();
}

template<typename Derived, typename V>
static inline void xorPixelUnchecked(RasterDrawMethods<Derived> &self, V Derived::*video, Vec2<int> pos)
{
	pixel &px = (static_cast<Derived &>(self).*video)[pos];
	auto const c = RGB<uint8_t>::Unpack(px);
	if (2 * c.Red + 3 * c.Green + c.Blue < 512)
		px = 0xC0C0C0_rgb .Pack();
	else
		px = 0x404040_rgb .Pack();
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::DrawPixel(Vec2<int> pos, RGB<uint8_t> colour)
{
	if (clipRect().Contains(pos))
		drawPixelUnchecked(*this, &Derived::video, pos, colour);
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::BlendPixel(Vec2<int> pos, RGBA<uint8_t> colour)
{
	if (clipRect().Contains(pos))
		blendPixelUnchecked(*this, &Derived::video, pos, colour);
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::AddPixel(Vec2<int> pos, RGBA<uint8_t> colour)
{
	if (clipRect().Contains(pos))
	{
		pixel &px = (static_cast<Derived &>(*this).video)[pos];
		px = RGB<uint8_t>::Unpack(px).Add(colour).Pack();
	}
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::XorPixel(Vec2<int> pos)
{
	if (clipRect().Contains(pos))
		xorPixelUnchecked(*this, &Derived::video, pos);
}

template<typename Derived>
void RasterDrawMethods<Derived>::DrawLine(Vec2<int> pos1, Vec2<int> pos2, RGB<uint8_t> colour)
{
	RasterizeLine<false>(pos1, pos2, [this, colour](Vec2<int> pos) {
		DrawPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendLine(Vec2<int> pos1, Vec2<int> pos2, RGBA<uint8_t> colour)
{
	RasterizeLine<false>(pos1, pos2, [this, colour](Vec2<int> pos) {
		BlendPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::AddLine(Vec2<int> pos1, Vec2<int> pos2, RGBA<uint8_t> colour)
{
	RasterizeLine<false>(pos1, pos2, [this, colour](Vec2<int> pos) {
		AddPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorLine(Vec2<int> pos1, Vec2<int> pos2)
{
	RasterizeLine<false>(pos1, pos2, [this](Vec2<int> pos) {
		XorPixel(pos);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::DrawRect(Rect<int> rect, RGB<uint8_t> colour)
{
	RasterizeRect(rect, [this, colour](Vec2<int> pos) {
		DrawPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendRect(Rect<int> rect, RGBA<uint8_t> colour)
{
	RasterizeRect(rect, [this, colour](Vec2<int> pos) {
		BlendPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorDottedRect(Rect<int> rect)
{
	RasterizeDottedRect(rect, [this](Vec2<int> pos) {
		XorPixel(pos);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::DrawFilledRect(Rect<int> rect, RGB<uint8_t> colour)
{
	rect &= clipRect();
	pixel packed = colour.Pack();
	auto &video = static_cast<Derived &>(*this).video;
	if (rect)
		for (int y = rect.TopLeft.Y; y <= rect.BottomRight.Y; y++)
			std::fill_n(video.RowIterator(Vec2(rect.TopLeft.X, y)), rect.Size().X, packed);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendFilledRect(Rect<int> rect, RGBA<uint8_t> colour)
{
	for (auto pos : rect & clipRect())
		blendPixelUnchecked(*this, &Derived::video, pos, colour);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendEllipse(Vec2<int> center, Vec2<int> size, RGBA<uint8_t> colour)
{
	RasterizeEllipsePoints(Vec2(float(size.X * size.X), float(size.Y * size.Y)), [this, center, colour](Vec2<int> delta) {
		BlendPixel(center + delta, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendFilledEllipse(Vec2<int> center, Vec2<int> size, RGBA<uint8_t> colour)
{
	RasterizeEllipseRows(Vec2(float(size.X * size.X), float(size.Y * size.Y)), [this, center, colour](int xLim, int dy) {
		for (auto pos : clipRect() & RectBetween(center + Vec2(-xLim, dy), center + Vec2(xLim, dy)))
			blendPixelUnchecked(*this, &Derived::video, pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendImage(pixel const *data, uint8_t alpha, Rect<int> rect)
{
	BlendImage(data, alpha, rect, rect.Size().X);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendImage(pixel const *data, uint8_t alpha, Rect<int> rect, size_t rowStride)
{
	auto origin = rect.TopLeft;
	rect &= clipRect();
	if (alpha == 0xFF)
	{
		auto &video = static_cast<Derived &>(*this).video;
		for (int y = rect.TopLeft.Y; y <= rect.BottomRight.Y; y++)
			std::copy_n(
				data + (rect.TopLeft.X - origin.X) + (y - origin.Y) * rowStride,
				rect.Size().X,
				video.RowIterator(Vec2(rect.TopLeft.X, y))
			);
	}
	else
	{
		for (auto pos : rect)
		{
			pixel const px = data[(pos.X - origin.X) + (pos.Y - origin.Y) * rowStride];
			blendPixelUnchecked(*this, &Derived::video, pos, RGB<uint8_t>::Unpack(px).WithAlpha(alpha));
		}
	}
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorImage(unsigned char const *data, Rect<int> rect)
{
	XorImage(data, rect, rect.Size().X);
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorImage(unsigned char const *data, Rect<int> rect, size_t rowStride)
{
	auto origin = rect.TopLeft;
	rect &= clipRect();
	for (auto pos : rect)
		if (data[(pos.X - origin.X) + (pos.Y - origin.Y) * rowStride])
			xorPixelUnchecked(*this, &Derived::video, pos);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendRGBAImage(pixel_rgba const *data, Rect<int> rect)
{
	BlendRGBAImage(data, rect, rect.Size().X);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendRGBAImage(pixel_rgba const *data, Rect<int> rect, size_t rowStride)
{
	auto origin = rect.TopLeft;
	rect &= clipRect();
	for (auto pos : rect)
	{
		pixel const px = data[(pos.X - origin.X) + (pos.Y - origin.Y) * rowStride];
		blendPixelUnchecked(*this, &Derived::video, pos, RGBA<uint8_t>::Unpack(px));
	}
}

template<typename Derived>
int RasterDrawMethods<Derived>::BlendChar(Vec2<int> pos, String::value_type ch, RGBA<uint8_t> colour)
{
	FontReader reader(ch);
	auto const rect = RectSized(Vec2(0, -2), Vec2(reader.GetWidth(), FONT_H));
	for (auto off : rect.template Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		BlendPixel(pos + off, colour.NoAlpha().WithAlpha(reader.NextPixel() * colour.Alpha / 3));
	return reader.GetWidth();
}

template<typename Derived>
int RasterDrawMethods<Derived>::AddChar(Vec2<int> pos, String::value_type ch, RGBA<uint8_t> colour)
{
	FontReader reader(ch);
	RGB<uint8_t> const c = colour.NoAlpha();
	auto const rect = RectSized(Vec2(0, -2), Vec2(reader.GetWidth(), FONT_H));
	for (auto off : rect.template Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		AddPixel(pos + off, c.WithAlpha(reader.NextPixel() * colour.Alpha / 3));
	return reader.GetWidth();
}

template<typename Derived>
Vec2<int> RasterDrawMethods<Derived>::BlendText(Vec2<int> orig_pos, String const &str, RGBA<uint8_t> orig_colour)
{
	bool underline = false;
	bool invert = false;
	RGB<uint8_t> colour = orig_colour.NoAlpha();
	uint8_t alpha = orig_colour.Alpha;
	Vec2<int> pos = orig_pos;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			pos.X = orig_pos.X;
			pos.Y += FONT_H;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i + 3)
				break;
			colour.Red = str[i + 1];
			colour.Green = str[i + 2];
			colour.Blue = str[i + 3];
			i += 3;
		}
		else if (str[i] == '\x0E')
		{
			colour = orig_colour.NoAlpha();
		}
		else if (str[i] == '\x01')
		{
			invert = !invert;
			colour = colour.Inverse();
		}
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			bool colourCode = true;
			switch (str[i + 1])
			{
			case 'U': underline = !underline; colourCode = false; break;
			case 'w': colour = 0xFFFFFF_rgb; break;
			case 'g': colour = 0xC0C0C0_rgb; break;
			case 'o': colour = 0xFFD820_rgb; break;
			case 'r': colour = 0xFF0000_rgb; break;
			case 'l': colour = 0xFF4B4B_rgb; break;
			case 'b': colour = 0x0000FF_rgb; break;
			case 't': colour = 0x20AAFF_rgb; break;
			case 'u': colour = 0x9353D3_rgb; break;
			}
			if (colourCode && invert)
				colour = colour.Inverse();
			i++;
		}
		else
		{
			int dx = BlendChar(pos, str[i], colour.WithAlpha(alpha));
			if (underline)
				for (int i = 0; i < dx; i++)
					BlendPixel(pos + Vec2(i, FONT_H), colour.WithAlpha(alpha));
			pos.X += dx;
		}
	}
	return pos - orig_pos;
}

template<typename Derived>
Vec2<int> RasterDrawMethods<Derived>::BlendTextOutline(Vec2<int> pos, String const &str, RGBA<uint8_t> colour)
{
	BlendText(pos + Vec2(-1, -1), str, 0x000000_rgb .WithAlpha(0x78));
	BlendText(pos + Vec2(-1, +1), str, 0x000000_rgb .WithAlpha(0x78));
	BlendText(pos + Vec2(+1, -1), str, 0x000000_rgb .WithAlpha(0x78));
	BlendText(pos + Vec2(+1, +1), str, 0x000000_rgb .WithAlpha(0x78));

	return BlendText(pos, str, colour);
}

template<typename Derived>
void RasterDrawMethods<Derived>::Clear()
{
	auto &video = static_cast<Derived &>(*this).video;
	std::fill_n(video.data(), video.Size().X * video.Size().Y, 0x000000_rgb .Pack());
}

template<typename Derived>
int RasterDrawMethods<Derived>::CharWidth(String::value_type ch)
{
	return FontReader(ch).GetWidth();
}

template<typename Derived>
Vec2<int> RasterDrawMethods<Derived>::TextSize(String const &str)
{
	Vec2<int> size = Vec2(0, FONT_H - 2);
	int curX = 0; // characters have 1px of spacing between them
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			size.X = std::max(curX, size.X);
			size.Y += FONT_H;
			curX = 0;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i + 3)
				break;
			i += 3;
		}
		else if (str[i] == '\x0E')
			continue;
		else if (str[i] == '\x01')
			continue;
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			i++;
		}
		else
			curX += CharWidth(str[i]);
	}
	size.X = std::max(curX, size.X);
	return size;
}

template<typename Derived>
String::const_iterator RasterDrawMethods<Derived>::TextFit(String const &str, int width)
{
	int curX = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
			curX = 0;
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i + 3)
				break;
			i += 3;
		}
		else if (str[i] == '\x0E')
			continue;
		else if (str[i] == '\x01')
			continue;
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			i++;
		}
		else
		{
			int dx = CharWidth(str[i]);
			if (curX + dx / 2 >= width)
				return str.begin() + i;
			curX += dx;
		}
	}
	return str.end();
}

#undef clipRect
