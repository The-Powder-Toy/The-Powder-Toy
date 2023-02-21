#include "common/RasterGeometry.h"
#include "FontReader.h"
#include "RasterDrawMethods.h"
#include <cmath>
#include <cstring>

template<typename Derived>
int RasterDrawMethods<Derived>::drawtext_outline(int x, int y, const String &s, int r, int g, int b, int a)
{
	drawtext(x-1, y-1, s, 0, 0, 0, 120);
	drawtext(x+1, y+1, s, 0, 0, 0, 120);

	drawtext(x-1, y+1, s, 0, 0, 0, 120);
	drawtext(x+1, y-1, s, 0, 0, 0, 120);

	return drawtext(x, y, s, r, g, b, a);
}

template<typename Derived>
int RasterDrawMethods<Derived>::drawtext(int x, int y, const String &str, int r, int g, int b, int a)
{
	if(!str.size())
		return 0;

	bool underline = false;
	int invert = 0;
	int oR = r, oG = g, oB = b;
	int characterX = x, characterY = y;
	int startX = characterX;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			characterX = startX;
			characterY += FONT_H;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i+3)
				break;
			oR = r;
			oG = g;
			oB = b;
			r = (unsigned char)str[i + 1];
			g = (unsigned char)str[i + 2];
			b = (unsigned char)str[i + 3];
			i += 3;
		}
		else if (str[i] == '\x0E')
		{
			r = oR;
			g = oG;
			b = oB;
		}
		else if (str[i] == '\x01')
		{
			invert = !invert;
			r = 255-r;
			g = 255-g;
			b = 255-b;
		}
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			auto colorCode = false;
			switch (str[i + 1])
			{
			case 'U':                    underline = !underline; break;
			case 'w': r = 255; g = 255; b = 255; colorCode = true; break;
			case 'g': r = 192; g = 192; b = 192; colorCode = true; break;
			case 'o': r = 255; g = 216; b =  32; colorCode = true; break;
			case 'r': r = 255; g =   0; b =   0; colorCode = true; break;
			case 'l': r = 255; g =  75; b =  75; colorCode = true; break;
			case 'b': r =   0; g =   0; b = 255; colorCode = true; break;
			case 't': b = 255; g = 170; r =  32; colorCode = true; break;
			case 'u': r = 147; g =  83; b = 211; colorCode = true; break;
			}
			if (colorCode && invert)
			{
				r = 255-r;
				g = 255-g;
				b = 255-b;
			}
			i++;
		}
		else
		{
			auto newCharacterX = drawchar(characterX, characterY, str[i], r, g, b, a);
			if (underline)
			{
				for (int i = characterX; i < newCharacterX; ++i)
				{
					blendpixel(i, y + FONT_H, r, g, b, a);
				}
			}
			characterX = newCharacterX;
		}
	}
	return x;
}

template<typename Derived>
int RasterDrawMethods<Derived>::drawchar(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			blendpixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

template<typename Derived>
int RasterDrawMethods<Derived>::addchar(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			addpixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

template<typename Derived>
void RasterDrawMethods<Derived>::xor_pixel(int x, int y)
{
	int c;
	if constexpr (Derived::DoClipCheck)
	{
		if (!(static_cast<Derived *>(this))->clip.Contains(Vec2<int>(x, y)))
			return;
	}
	else
	{
		if (!Derived::VIDRES.OriginRect().Contains(Vec2<int>(x, y)))
			return;
	}
	c = (static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x];
	c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
	if (c<512)
		(static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x] = PIXPACK(0xC0C0C0);
	else
		(static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x] = PIXPACK(0x404040);
}

template<typename Derived>
void RasterDrawMethods<Derived>::blendpixel(int x, int y, int r, int g, int b, int a)
{
	pixel t;
	if constexpr (Derived::DoClipCheck)
	{
		if (!(static_cast<Derived *>(this))->clip.Contains(Vec2<int>(x, y)))
			return;
	}
	else
	{
		if (!Derived::VIDRES.OriginRect().Contains(Vec2<int>(x, y)))
			return;
	}
	if (a!=255)
	{
		t = (static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
	}
	(static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x] = PIXRGB(r,g,b);
}

template<typename Derived>
void RasterDrawMethods<Derived>::addpixel(int x, int y, int r, int g, int b, int a)
{
	pixel t;
	if constexpr (Derived::DoClipCheck)
	{
		if (!(static_cast<Derived *>(this))->clip.Contains(Vec2<int>(x, y)))
			return;
	}
	else
	{
		if (!Derived::VIDRES.OriginRect().Contains(Vec2<int>(x, y)))
			return;
	}
	t = (static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x];
	r = (a*r + 255*PIXR(t)) >> 8;
	g = (a*g + 255*PIXG(t)) >> 8;
	b = (a*b + 255*PIXB(t)) >> 8;
	if (r>255)
		r = 255;
	if (g>255)
		g = 255;
	if (b>255)
		b = 255;
	(static_cast<Derived *>(this))->vid[y*(Derived::VIDXRES)+x] = PIXRGB(r,g,b);
}

template<typename Derived>
void RasterDrawMethods<Derived>::xor_line(int x1, int y1, int x2, int y2)
{
	RasterizeLine<false>(Vec2<int>(x1, y1), Vec2<int>(x2, y2),
		[this](Vec2<int> p) { xor_pixel(p.X, p.Y); });
}

template<typename Derived>
void RasterDrawMethods<Derived>::xor_rect(int x, int y, int w, int h)
{
	RasterizeDottedRect(RectSized(Vec2<int>(x, y), Vec2<int>(w, h)),
		[this](Vec2<int> p) { xor_pixel(p.X, p.Y); });
}

template<typename Derived>
void RasterDrawMethods<Derived>::xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h)
{
	for(int x1 = 0; x1 < w; x1++)
	{
		for(int y1 = 0; y1 < h; y1++)
		{
			if(bitmap[y1*w+x1])
				xor_pixel(x+x1, y+y1);
		}
	}
}

template<typename Derived>
void RasterDrawMethods<Derived>::draw_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	RasterizeLine<false>(Vec2<int>(x1, y1), Vec2<int>(x2, y2),
		[this, r, g, b, a](Vec2<int> p) { blendpixel(p.X, p.Y, r, g, b, a); });
}

template<typename Derived>
void RasterDrawMethods<Derived>::drawrect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	RasterizeRect(RectSized(Vec2<int>(x, y), Vec2<int>(w, h)),
		[this, r, g, b, a](Vec2<int> p) { blendpixel(p.X, p.Y, r, g, b, a); });
}

template<typename Derived>
void RasterDrawMethods<Derived>::fillrect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i,j;
	for (j=0; j<h; j++)
		for (i=0; i<w; i++)
			blendpixel(x+i, y+j, r, g, b, a);
}

template<typename Derived>
void RasterDrawMethods<Derived>::drawcircle(int x, int y, int rx, int ry, int r, int g, int b, int a)
{
	RasterizeEllipsePoints(Vec2<float>(rx * rx, ry * ry),
		[=](Vec2<int> p) { blendpixel(x + p.X, y + p.Y, r, g, b, a); });
}

template<typename Derived>
void RasterDrawMethods<Derived>::fillcircle(int x, int y, int rx, int ry, int r, int g, int b, int a)
{
	RasterizeEllipseRows(Vec2<float>(rx * rx, ry * ry), [=](int xLim, int dy)
		{
			for (int dx = -xLim; dx <= xLim; dx++)
				blendpixel(x + dx, y + dy, r, g, b, a);
		});
}

template<typename Derived>
void RasterDrawMethods<Derived>::gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2)
{

}

template<typename Derived>
void RasterDrawMethods<Derived>::clearrect(int x, int y, int w, int h)
{
	int i;

	// TODO: change calls to clearrect to use sensible meanings of x, y, w, h then remove these 4 lines
	x += 1;
	y += 1;
	w -= 1;
	h -= 1;

	Rect<int> rect = RectSized(Vec2<int>(x, y), Vec2<int>(w, h));
	if constexpr (Derived::DoClipCheck)
		rect = (static_cast<Derived *>(this))->clip.Clamp(rect);
	else
		rect = Derived::VIDRES.OriginRect().Clamp(rect);

	x = rect.TopLeft.X;
	y = rect.TopLeft.Y;
	w = rect.Size().X;
	h = rect.Size().Y;

	if (w<0 || h<0)
		return;

	for (i=0; i<h; i++)
		memset((static_cast<Derived *>(this))->vid+(x+(Derived::VIDXRES)*(y+i)), 0, PIXELSIZE*w);
}

template<typename Derived>
void RasterDrawMethods<Derived>::draw_image(const pixel *img, int x, int y, int w, int h, int a)
{
	int startX = 0;
	if (!img)
		return;
	// Adjust height to prevent drawing off the bottom
	if (y + h > Derived::VIDYRES)
		h = ((Derived::VIDYRES)-y)-1;
	// Too big
	if (x + w > Derived::VIDXRES)
		return;

	// Starts off the top of the screen, adjust
	if (y < 0 && -y < h)
	{
		img += -y*w;
		h += y;
		y = 0;
	}
	// Starts off the left side of the screen, adjust
	if (x < 0 && -x < w)
	{
		startX = -x;
	}

	if (!h || y < 0 || !w)
		return;
	if (a >= 255)
		for (int j = 0; j < h; j++)
		{
			img += startX;
			for (int i = startX; i < w; i++)
			{
				if constexpr (Derived::DoClipCheck)
				{
					if ((static_cast<Derived *>(this))->clip.Contains(Vec2<int>(x + i, y + j)))
						(static_cast<Derived *>(this))->vid[(y+j)*(Derived::VIDXRES)+(x+i)] = *img;
				}
				else
				{
					(static_cast<Derived *>(this))->vid[(y+j)*(Derived::VIDXRES)+(x+i)] = *img;
				}
				img++;
			}
		}
	else
	{
		int r, g, b;
		for (int j = 0; j < h; j++)
		{
			img += startX;
			for (int i = startX; i < w; i++)
			{
				r = PIXR(*img);
				g = PIXG(*img);
				b = PIXB(*img);
				blendpixel(x+i, y+j, r, g, b, a);
				img++;
			}
		}
	}
}

template<typename Derived>
void RasterDrawMethods<Derived>::draw_image(const VideoBuffer * vidBuf, int x, int y, int a)
{
	draw_image(vidBuf->Buffer, x, y, vidBuf->Width, vidBuf->Height, a);
}
