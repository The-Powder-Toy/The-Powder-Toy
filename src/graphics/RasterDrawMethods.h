#pragma once
#include "common/String.h"

class VideoBuffer;

// The "Curiously Recurring Template Pattern" trick
template<typename Derived>
struct RasterDrawMethods
{
	int drawtext_outline(int x, int y, const String &s, int r, int g, int b, int a);
	int drawtext(int x, int y, const String &str, int r, int g, int b, int a);
	int drawchar(int x, int y, String::value_type c, int r, int g, int b, int a);
	int addchar(int x, int y, String::value_type c, int r, int g, int b, int a);
	void xor_pixel(int x, int y);
	void blendpixel(int x, int y, int r, int g, int b, int a);
	void addpixel(int x, int y, int r, int g, int b, int a);
	void xor_line(int x1, int y1, int x2, int y2);
	void xor_rect(int x, int y, int w, int h);
	void xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h);
	void draw_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
	void drawrect(int x, int y, int w, int h, int r, int g, int b, int a);
	void fillrect(int x, int y, int w, int h, int r, int g, int b, int a);
	void drawcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	void fillcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	void gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2);
	void clearrect(int x, int y, int w, int h);
	void draw_image(const pixel *img, int x, int y, int w, int h, int a);
	void draw_image(const VideoBuffer * vidBuf, int x, int y, int a);
};
