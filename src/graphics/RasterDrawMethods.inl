#include "font.h"
#include <cmath>

int PIXELMETHODS_CLASS::drawtext_outline(int x, int y, const char *s, int r, int g, int b, int a)
{
	drawtext(x-1, y-1, s, 0, 0, 0, 120);
	drawtext(x+1, y+1, s, 0, 0, 0, 120);
	
	drawtext(x-1, y+1, s, 0, 0, 0, 120);
	drawtext(x+1, y-1, s, 0, 0, 0, 120);
	
	return drawtext(x, y, s, r, g, b, a);
}

int PIXELMETHODS_CLASS::drawtext(int x, int y, const char *s, int r, int g, int b, int a)
{
	if(!strlen(s))
		return 0;

	int invert = 0;
	int oR = r, oG = g, oB = b;
	int characterX = x, characterY = y;
	int startX = characterX;
	for (; *s; s++)
	{
		if (*s == '\n')
		{
			characterX = startX;
			characterY += FONT_H+2;
		}
		else if (*s == '\x0F')
		{
			if(!s[1] || !s[2] || !s[3]) break;
			oR = r;
			oG = g;
			oB = b;
			r = (unsigned char)s[1];
			g = (unsigned char)s[2];
			b = (unsigned char)s[3];
			s += 3;
		}
		else if (*s == '\x0E')
		{
			r = oR;
			g = oG;
			b = oB;
		}
		else if (*s == '\x01')
		{
			invert = !invert;
			r = 255-r;
			g = 255-g;
			b = 255-b;
		}
		else if (*s == '\b')
		{
			if(!s[1]) break;
			switch (s[1])
			{
			case 'w':
				r = g = b = 255;
				break;
			case 'g':
				r = g = b = 192;
				break;
			case 'o':
				r = 255;
				g = 216;
				b = 32;
				break;
			case 'r':
				r = 255;
				g = b = 0;
				break;
			case 'l':
				r = 255;
				g = b = 75;
				break;
			case 'b':
				r = g = 0;
				b = 255;
				break;
			case 't':
				b = 255;
				g = 170;
				r = 32;
				break;
			}
			if(invert)
			{
				r = 255-r;
				g = 255-g;
				b = 255-b;
			}
			s++;
		}
		else
		{
			characterX = drawchar(characterX, characterY, *(unsigned char *)s, r, g, b, a);
		}
	}
	return x;
}

int PIXELMETHODS_CLASS::drawtext(int x, int y, std::string s, int r, int g, int b, int a)
{
	return drawtext(x, y, s.c_str(), r, g, b, a);
}

int PIXELMETHODS_CLASS::drawchar(int x, int y, int c, int r, int g, int b, int a)
{
	int i, j, w, bn = 0, ba = 0;
	unsigned char *rp = font_data + font_ptrs[c];
	w = *(rp++);
	for (j=0; j<FONT_H; j++)
		for (i=0; i<w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			blendpixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

int PIXELMETHODS_CLASS::addchar(int x, int y, int c, int r, int g, int b, int a)
{
	int i, j, w, bn = 0, ba = 0;
	unsigned char *rp = font_data + font_ptrs[c];
	w = *(rp++);
	for (j=0; j<FONT_H; j++)
		for (i=0; i<w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			{
			addpixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			}
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

TPT_INLINE void PIXELMETHODS_CLASS::xor_pixel(int x, int y)
{
	int c;
	if (x<0 || y<0 || x>=XRES || y>=YRES)
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
	if (x<0 || y<0 || x>=VIDXRES || y>=VIDYRES)
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
	if (x<0 || y<0 || x>=VIDXRES || y>=VIDYRES)
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
	if (w<0 || h<0)
		return;

	for (i=0; i<h; i++)
		memset(vid+(x+(VIDXRES)*(y+i)), 0, PIXELSIZE*w);
}

void PIXELMETHODS_CLASS::draw_image(pixel *img, int x, int y, int w, int h, int a)
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

void PIXELMETHODS_CLASS::draw_image(const VideoBuffer & vidBuf, int x, int y, int a)
{
	draw_image(vidBuf.Buffer, x, y, vidBuf.Width, vidBuf.Height, a);
}

void PIXELMETHODS_CLASS::draw_image(VideoBuffer * vidBuf, int x, int y, int a)
{
	draw_image(vidBuf->Buffer, x, y, vidBuf->Width, vidBuf->Height, a);
}
