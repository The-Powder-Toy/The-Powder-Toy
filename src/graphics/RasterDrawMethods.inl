#include <cmath>
#include "FontReader.h"

int PIXELMETHODS_CLASS::drawtext_outline(int x, int y, const String &s, int r, int g, int b, int a)
{
	drawtext(x-1, y-1, s, 0, 0, 0, 120);
	drawtext(x+1, y+1, s, 0, 0, 0, 120);

	drawtext(x-1, y+1, s, 0, 0, 0, 120);
	drawtext(x+1, y-1, s, 0, 0, 0, 120);

	return drawtext(x, y, s, r, g, b, a);
}

int PIXELMETHODS_CLASS::drawtext(int x, int y, const String &str, int r, int g, int b, int a)
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

int PIXELMETHODS_CLASS::drawchar(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			blendpixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

int PIXELMETHODS_CLASS::addchar(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			addpixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

TPT_INLINE void PIXELMETHODS_CLASS::xor_pixel(int x, int y)
{
	int c;
#ifdef DO_CLIPCHECK
	if (x<clipx1 || y<clipy1 || x>=clipx2 || y>=clipy2)
#else
	if (x<0 || y<0 || x>=VIDXRES || y>=VIDYRES)
#endif
		return;
	c = vid[y*(VIDXRES)+x];
	c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
	if (c<512)
		vid[y*(VIDXRES)+x] = PIXPACK(0xC0C0C0);
	else
		vid[y*(VIDXRES)+x] = PIXPACK(0x404040);
}

void PIXELMETHODS_CLASS::blendpixel(int x, int y, int r, int g, int b, int a)
{
	pixel t;
#ifdef DO_CLIPCHECK
	if (x<clipx1 || y<clipy1 || x>=clipx2 || y>=clipy2)
#else
	if (x<0 || y<0 || x>=VIDXRES || y>=VIDYRES)
#endif
		return;
	if (a!=255)
	{
		t = vid[y*(VIDXRES)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
	}
	vid[y*(VIDXRES)+x] = PIXRGB(r,g,b);
}

void PIXELMETHODS_CLASS::addpixel(int x, int y, int r, int g, int b, int a)
{
	pixel t;
#ifdef DO_CLIPCHECK
	if (x<clipx1 || y<clipy1 || x>=clipx2 || y>=clipy2)
#else
	if (x<0 || y<0 || x>=VIDXRES || y>=VIDYRES)
#endif
		return;
	t = vid[y*(VIDXRES)+x];
	r = (a*r + 255*PIXR(t)) >> 8;
	g = (a*g + 255*PIXG(t)) >> 8;
	b = (a*b + 255*PIXB(t)) >> 8;
	if (r>255)
		r = 255;
	if (g>255)
		g = 255;
	if (b>255)
		b = 255;
	vid[y*(VIDXRES)+x] = PIXRGB(r,g,b);
}

void PIXELMETHODS_CLASS::xor_line(int x1, int y1, int x2, int y2)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
	float e, de;
	if (cp)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	e = 0.0f;
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (cp)
			xor_pixel(y, x);
		else
			xor_pixel(x, y);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

void PIXELMETHODS_CLASS::xor_rect(int x, int y, int w, int h)
{
	int i;
	for (i=0; i<w; i+=2)
	{
		xor_pixel(x+i, y);
	}
	if (h != 1)
	{
		if (h%2 == 1) i = 2;
		else i = 1;
		for (; i<w; i+=2)
		{
			xor_pixel(x+i, y+h-1);
		}
	}

	for (i=2; i<h; i+=2)
	{
		xor_pixel(x, y+i);
	}
	if (w != 1)
	{
		if (w%2 == 1) i = 2;
		else i = 1;
		for (; i<h-1; i+=2)
		{
			xor_pixel(x+w-1, y+i);
		}
	}
}

void PIXELMETHODS_CLASS::xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h)
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

void PIXELMETHODS_CLASS::draw_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
	float e, de;
	if (cp)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	e = 0.0f;
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (cp)
			blendpixel(y, x, r, g, b, a);
		else
			blendpixel(x, y, r, g, b, a);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

void PIXELMETHODS_CLASS::drawrect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i;
	w--;
	h--;
	for (i=0; i<=w; i++)
	{
		blendpixel(x+i, y, r, g, b, a);
		blendpixel(x+i, y+h, r, g, b, a);
	}
	for (i=1; i<h; i++)
	{
		blendpixel(x, y+i, r, g, b, a);
		blendpixel(x+w, y+i, r, g, b, a);
	}
}

void PIXELMETHODS_CLASS::fillrect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i,j;
	for (j=0; j<h; j++)
		for (i=0; i<w; i++)
			blendpixel(x+i, y+j, r, g, b, a);
}

void PIXELMETHODS_CLASS::drawcircle(int x, int y, int rx, int ry, int r, int g, int b, int a)
{
	int yTop = ry, yBottom, i, j;
	if (!rx)
	{
		for (j = -ry; j <= ry; j++)
			blendpixel(x, y+j, r, g, b, a);
		return;
	}
	for (i = 0; i <= rx; i++) {
		yBottom = yTop;
		while (pow(i-rx,2.0)*pow(ry,2.0) + pow(yTop-ry,2.0)*pow(rx,2.0) <= pow(rx,2.0)*pow(ry,2.0))
			yTop++;
		if (yBottom != yTop)
			yTop--;
		for (int j = yBottom; j <= yTop; j++)
		{
			blendpixel(x+i-rx, y+j-ry, r, g, b, a);
			if (i != rx)
				blendpixel(x-i+rx, y+j-ry, r, g, b, a);
			if (j != ry)
			{
				blendpixel(x+i-rx, y-j+ry, r, g, b, a);
				if (i != rx)
					blendpixel(x-i+rx, y-j+ry, r, g, b, a);
			}
		}
	}
}

void PIXELMETHODS_CLASS::fillcircle(int x, int y, int rx, int ry, int r, int g, int b, int a)
{
	int yTop = ry+1, yBottom, i, j;
	if (!rx)
	{
		for (j = -ry; j <= ry; j++)
			blendpixel(x, y+j, r, g, b, a);
		return;
	}
	for (i = 0; i <= rx; i++)
	{
		while (pow(i-rx,2.0)*pow(ry,2.0) + pow(yTop-ry,2.0)*pow(rx,2.0) <= pow(rx,2.0)*pow(ry,2.0))
			yTop++;
		yBottom = 2*ry - yTop;
		for (int j = yBottom+1; j < yTop; j++)
		{
			blendpixel(x+i-rx, y+j-ry, r, g, b, a);
			if (i != rx)
				blendpixel(x-i+rx, y+j-ry, r, g, b, a);
		}
	}
}

void PIXELMETHODS_CLASS::gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2)
{

}

void PIXELMETHODS_CLASS::clearrect(int x, int y, int w, int h)
{
	int i;

	// TODO: change calls to clearrect to use sensible meanings of x, y, w, h then remove these 4 lines
	x += 1;
	y += 1;
	w -= 1;
	h -= 1;

#ifdef DO_CLIPCHECK
	if (x+w > clipx2) w = clipx2-x;
	if (y+h > clipy2) h = clipy2-y;
	if (x<clipx1)
	{
		w += x - clipx1;
		x = clipx1;
	}
	if (y<clipy1)
	{
		h += y - clipy1;
		y = clipy1;
	}
#else
	if (x+w > VIDXRES) w = VIDXRES-x;
	if (y+h > VIDYRES) h = VIDYRES-y;
	if (x<0)
	{
		w += x;
		x = 0;
	}
	if (y<0)
	{
		h += y;
		y = 0;
	}
#endif
	if (w<0 || h<0)
		return;

	for (i=0; i<h; i++)
		memset(vid+(x+(VIDXRES)*(y+i)), 0, PIXELSIZE*w);
}

void PIXELMETHODS_CLASS::draw_image(const pixel *img, int x, int y, int w, int h, int a)
{
	int startX = 0;
	if (!img)
		return;
	// Adjust height to prevent drawing off the bottom
	if (y + h > VIDYRES)
		h = ((VIDYRES)-y)-1;
	// Too big
	if (x + w > VIDXRES)
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
#ifdef DO_CLIPCHECK
				if (!(x+i<clipx1 || y+j<clipy1 || x+i>=clipx2 || y+j>=clipy2))
#endif
				vid[(y+j)*(VIDXRES)+(x+i)] = *img;
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

void PIXELMETHODS_CLASS::draw_image(const VideoBuffer * vidBuf, int x, int y, int a)
{
	draw_image(vidBuf->Buffer, x, y, vidBuf->Width, vidBuf->Height, a);
}
