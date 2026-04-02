#pragma once
#include "common/String.h"
#include "common/Vec2.h"
#include "graphics/Pixel.h"

class VideoBuffer;

// This is a mixin that adds methods to the Derived class, using the "Curiously
// Recurring Template Pattern" trick.
template<typename Derived>
struct RasterDrawMethods
{
	void DrawPixel(Vec2<int>, RGB);
	void BlendPixel(Vec2<int>, RGBA);
	void AddPixel(Vec2<int>, RGBA);
	void AddFirePixel(Vec2<int>, RGB, int fireAlpha);
	void XorPixel(Vec2<int>);

	void DrawLine(Vec2<int>, Vec2<int>, RGB);
	void BlendLine(Vec2<int>, Vec2<int>, RGBA);
	void AddLine(Vec2<int>, Vec2<int>, RGBA);
	void XorLine(Vec2<int>, Vec2<int>);

	void DrawRect(Rect<int>, RGB);
	void BlendRect(Rect<int>, RGBA);

	void XorDottedRect(Rect<int>);

	void DrawFilledRect(Rect<int>, RGB);
	void BlendFilledRect(Rect<int>, RGBA);

	void BlendEllipse(Vec2<int> center, Vec2<int> size, RGBA);

	void BlendFilledEllipse(Vec2<int> center, Vec2<int> size, RGBA);

	void BlendImage(pixel const *, uint8_t alpha, Rect<int>);
	void BlendImage(pixel const *, uint8_t alpha, Rect<int>, size_t rowStride);
	void XorImage(unsigned char const *, Rect<int>);
	void XorImage(unsigned char const *, Rect<int>, size_t rowStride);

	void BlendRGBAImage(pixel_rgba const *, Rect<int>);
	void BlendRGBAImage(pixel_rgba const *, Rect<int>, size_t rowStride);

	// Returns width of character
	int BlendChar(Vec2<int>, String::value_type, RGBA);
	int AddChar(Vec2<int>, String::value_type, RGBA);

	// Returns the offset between the first character and the
	// would-be-next character
	Vec2<int> BlendText(Vec2<int>, String const &, RGBA);

	Vec2<int> BlendTextOutline(Vec2<int>, String const &, RGBA);

	static int CharWidth(String::value_type);
	// Considers the first line to be FONT_H-2 tall with successive lines adding
	// FONT_H each
	static Vec2<int> TextSize(String const &);
	// Return iterator to the end of an initial portion of text that fits in
	// the given width
	static String::const_iterator TextFit(String const &, int width);

	void Clear();
};
