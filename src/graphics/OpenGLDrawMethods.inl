#include "../data/font.h"
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
	bool invert = false;
	if(!strlen(s))
		return 0;
	int oR = r, oG = g, oB = b;
	int width, height;
	Graphics::textsize(s, width, height);
	VideoBuffer texture(width, height);
	int characterX = 0, characterY = 0;
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
			characterX = texture.SetCharacter(characterX, characterY, *(unsigned char *)s, r, g, b, a);
		}
	}
	glEnable(GL_TEXTURE_2D);

	//Generate texture
	glBindTexture(GL_TEXTURE_2D, textTexture);

	//Draw texture
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width, texture.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture.Buffer);

    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.Width, texture.Height, GL_BGRA, GL_UNSIGNED_BYTE, texture.Buffer);
	glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2f(x, y);
    glTexCoord2d(1, 0);
    glVertex2f(x+texture.Width, y);
    glTexCoord2d(1, 1);
    glVertex2f(x+texture.Width, y+texture.Height);
    glTexCoord2d(0, 1);
    glVertex2f(x, y+texture.Height);
    glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return x;
}

int PIXELMETHODS_CLASS::drawtext(int x, int y, std::string s, int r, int g, int b, int a)
{
	return drawtext(x, y, s.c_str(), r, g, b, a);
}

int PIXELMETHODS_CLASS::drawchar(int x, int y, int c, int r, int g, int b, int a)
{
	unsigned char *rp = font_data + font_ptrs[c];
	int w = *(rp++);
	VideoBuffer texture(w, 12);
	texture.SetCharacter(0, 0, c, r, g, b, a);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textTexture);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width, texture.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture.Buffer);
	glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2f(x, y);
    glTexCoord2d(1, 0);
    glVertex2f(x+texture.Width, y);
    glTexCoord2d(1, 1);
    glVertex2f(x+texture.Width, y+texture.Height);
    glTexCoord2d(0, 1);
    glVertex2f(x, y+texture.Height);
    glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return x + w;
}

int PIXELMETHODS_CLASS::addchar(int x, int y, int c, int r, int g, int b, int a)
{
	unsigned char *rp = font_data + font_ptrs[c];
	int w = *(rp++);
	VideoBuffer texture(w, 12);
	texture.AddCharacter(0, 0, c, r, g, b, a);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textTexture);
	glBlendFunc(GL_ONE, GL_ONE);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width, texture.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture.Buffer);
	glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2f(x, y);
    glTexCoord2d(1, 0);
    glVertex2f(x+texture.Width, y);
    glTexCoord2d(1, 1);
    glVertex2f(x+texture.Width, y+texture.Height);
    glTexCoord2d(0, 1);
    glVertex2f(x, y+texture.Height);
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return x + w;
}

TPT_INLINE void PIXELMETHODS_CLASS::xor_pixel(int x, int y)
{
	//OpenGL doesn't support single pixel manipulation, there are ways around it, but with poor performance
}

void PIXELMETHODS_CLASS::blendpixel(int x, int y, int r, int g, int b, int a)
{
	//OpenGL doesn't support single pixel manipulation, there are ways around it, but with poor performance
}

void PIXELMETHODS_CLASS::addpixel(int x, int y, int r, int g, int b, int a)
{
	//OpenGL doesn't support single pixel manipulation, there are ways around it, but with poor performance
}

void PIXELMETHODS_CLASS::xor_line(int x, int y, int x2, int y2)
{
	glEnable(GL_COLOR_LOGIC_OP);
	//glEnable(GL_LINE_SMOOTH);
	glLogicOp(GL_XOR);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x2, y2);
	glEnd();
	glDisable(GL_COLOR_LOGIC_OP);
}

void PIXELMETHODS_CLASS::xor_rect(int x, int y, int width, int height)
{
	glEnable(GL_COLOR_LOGIC_OP);
	//glEnable(GL_LINE_SMOOTH);
	glLogicOp(GL_XOR);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2i(x, y);
	glVertex2i(x+width, y);
	glVertex2i(x+width, y+height);
	glVertex2i(x, y+height);
	glVertex2i(x, y);
	glEnd();
	glDisable(GL_COLOR_LOGIC_OP);
}

void PIXELMETHODS_CLASS::xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h)
{
	//glEnable(GL_COLOR_LOGIC_OP);
	//glLogicOp(GL_XOR);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textTexture);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);

	glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2f(x, y);
    glTexCoord2d(1, 0);
    glVertex2f(x+w, y);
    glTexCoord2d(1, 1);
    glVertex2f(x+w, y+h);
    glTexCoord2d(0, 1);
    glVertex2f(x, y+h);
    glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_COLOR_LOGIC_OP);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void PIXELMETHODS_CLASS::draw_line(int x, int y, int x2, int y2, int r, int g, int b, int a)
{
	a = 255;
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x2, y2);
	glEnd();
}

void PIXELMETHODS_CLASS::drawrect(int x, int y, int width, int height, int r, int g, int b, int a)
{
	float fx = float(x)+0.5f;
	float fy = float(y)+0.5f;
	float fwidth = width-1.0f;
	float fheight = height-1.0f;
	//x++;
	//y++;
	//height-=2;
	//width-=2;
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_STRIP);
	glVertex2f(fx, fy);
	glVertex2f(fx+fwidth, fy);
	glVertex2f(fx+fwidth, fy+fheight);
	glVertex2f(fx, fy+fheight); //+1 is a hack to prevent squares from missing their corners, will make smoothed lines look like SHIT
	glVertex2f(fx, fy);
	glEnd();
}

void PIXELMETHODS_CLASS::fillrect(int x, int y, int width, int height, int r, int g, int b, int a)
{
	/*x++;
	y++;
	width-=1;
	height-=1;*/

	glColor4ub(r, g, b, a);
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x+width, y);
	glVertex2i(x+width, y+height);
	glVertex2i(x, y+height);
	glEnd();
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
	glBegin(GL_QUADS);
	glColor4ub(r, g, b, a);
	glVertex2i(x, y);
	glColor4ub(r2, g2, b2, a2);
	glVertex2i(x+width, y);
	glColor4ub(r2, g2, b2, a2);
	glVertex2i(x+width, y+height);
	glColor4ub(r, g, b, a);
	glVertex2i(x, y+height);
	glEnd();
}

void PIXELMETHODS_CLASS::clearrect(int x, int y, int width, int height)
{
	glColor4ub(0, 0, 0, 255);
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x+width, y);
	glVertex2i(x+width, y+height);
	glVertex2i(x, y+height);
	glEnd();
}

void PIXELMETHODS_CLASS::draw_image(pixel *img, int x, int y, int w, int h, int a)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textTexture);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
	glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2f(x, y);
    glTexCoord2d(1, 0);
    glVertex2f(x+w, y);
    glTexCoord2d(1, 1);
    glVertex2f(x+w, y+h);
    glTexCoord2d(0, 1);
    glVertex2f(x, y+h);
    glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void PIXELMETHODS_CLASS::draw_image(VideoBuffer * vidBuf, int x, int y, int a)
{
	draw_image(vidBuf->Buffer, x, y, vidBuf->Width, vidBuf->Height, a);
}
