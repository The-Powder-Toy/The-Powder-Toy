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
	void DrawPixel(Vec2<int>, RGB<uint8_t>);
	void BlendPixel(Vec2<int>, RGBA<uint8_t>);
	void AddPixel(Vec2<int>, RGBA<uint8_t>);
	void XorPixel(Vec2<int>);

	void DrawLine(Vec2<int>, Vec2<int>, RGB<uint8_t>);
	void BlendLine(Vec2<int>, Vec2<int>, RGBA<uint8_t>);
	void AddLine(Vec2<int>, Vec2<int>, RGBA<uint8_t>);
	void XorLine(Vec2<int>, Vec2<int>);

	void DrawRect(Rect<int>, RGB<uint8_t>);
	void BlendRect(Rect<int>, RGBA<uint8_t>);

	void XorDottedRect(Rect<int>);

	void DrawFilledRect(Rect<int>, RGB<uint8_t>);
	void BlendFilledRect(Rect<int>, RGBA<uint8_t>);

	void BlendEllipse(Vec2<int> center, Vec2<int> size, RGBA<uint8_t>);

	void BlendFilledEllipse(Vec2<int> center, Vec2<int> size, RGBA<uint8_t>);

	void BlendImage(pixel const *, uint8_t alpha, Rect<int>);
	void BlendImage(pixel const *, uint8_t alpha, Rect<int>, size_t rowStride);
	void XorImage(unsigned char const *, Rect<int>);
	void XorImage(unsigned char const *, Rect<int>, size_t rowStride);

	void BlendRGBAImage(pixel_rgba const *, Rect<int>);
	void BlendRGBAImage(pixel_rgba const *, Rect<int>, size_t rowStride);

	// Returns width of character
	int BlendChar(Vec2<int>, String::value_type, RGBA<uint8_t>);
	int AddChar(Vec2<int>, String::value_type, RGBA<uint8_t>);

	// Returns the offset between the first character and the
	// would-be-next character
	Vec2<int> BlendText(Vec2<int>, String const &, RGBA<uint8_t>);

	Vec2<int> BlendTextOutline(Vec2<int>, String const &, RGBA<uint8_t>);

	void Clear();

	[[deprecated("Use BlendTextOutline")]]
	int drawtext_outline(int x, int y, const String &, int r, int g, int b, int a);
	[[deprecated("Use BlendText")]]
	int drawtext(int x, int y, const String &, int r, int g, int b, int a);
	[[deprecated("Use BlendChar")]]
	int drawchar(int x, int y, String::value_type, int r, int g, int b, int a);
	[[deprecated("Use AddChar")]]
	int addchar(int x, int y, String::value_type, int r, int g, int b, int a);
	[[deprecated("Use XorPixel")]]
	void xor_pixel(int x, int y);
	[[deprecated("Use DrawPixel/BlendPixel")]]
	void blendpixel(int x, int y, int r, int g, int b, int a);
	[[deprecated("Use AddPixel")]]
	void addpixel(int x, int y, int r, int g, int b, int a);
	[[deprecated("Use XorLine")]]
	void xor_line(int x1, int y1, int x2, int y2);
	[[deprecated("Use XorDottedRect")]]
	void xor_rect(int x, int y, int w, int h);
	[[deprecated("Use XorImage")]]
	void xor_bitmap(unsigned char *bitmap, int x, int y, int w, int h);
	[[deprecated("Use DrawLine/BlendLine")]]
	void draw_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
	[[deprecated("Use DrawRect/BlendRect")]]
	void drawrect(int x, int y, int w, int h, int r, int g, int b, int a);
	[[deprecated("Use DrawFilledRect/BlendFilledRect")]]
	void fillrect(int x, int y, int w, int h, int r, int g, int b, int a);
	[[deprecated("Use BlendEllipse")]]
	void drawcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	[[deprecated("Use BlendFilledEllipse")]]
	void fillcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	[[deprecated("Use DrawFilledRect (beware off by 1)")]]
	void clearrect(int x, int y, int w, int h);
	[[deprecated("Use BlendImage")]]
	void draw_image(pixel const *, int x, int y, int w, int h, int a);
	[[deprecated("Use BlendImage")]]
	void draw_image(VideoBuffer const *, int x, int y, int a);
};
