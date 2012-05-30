#include "Graphics.h"
#include "font.h"

#ifdef OGLR

Graphics::Graphics():
sdl_scale(1)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//glOrtho(0, (XRES+BARSIZE)*sdl_scale, 0, (YRES+MENUSIZE)*sdl_scale, -1, 1);
	glOrtho(0, (XRES+BARSIZE)*sdl_scale, (YRES+MENUSIZE)*sdl_scale, 0, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	
	//glRasterPos2i(0, (YRES+MENUSIZE));
	glRasterPos2i(0, 0);
	glPixelZoom(1, 1);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//Texture for main UI
	glEnable(GL_TEXTURE_2D);
	
	glGenTextures(1, &vidBuf);
	glBindTexture(GL_TEXTURE_2D, vidBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, XRES+BARSIZE, YRES+MENUSIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenTextures(1, &textTexture);
	glBindTexture(GL_TEXTURE_2D, textTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glDisable(GL_TEXTURE_2D);
}

Graphics::~Graphics()
{
}

void Graphics::Clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Graphics::Finalise()
{
    glFlush();
}

int Graphics::drawtext(int x, int y, const char *s, int r, int g, int b, int a)
{
	if(!strlen(s))
		return 0;
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
		else if (*s == '\b')
		{
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

int Graphics::drawtext(int x, int y, std::string s, int r, int g, int b, int a)
{
	return drawtext(x, y, s.c_str(), r, g, b, a);
}

TPT_INLINE int Graphics::drawchar(int x, int y, int c, int r, int g, int b, int a)
{
	int i, j, w, bn = 0, ba = 0;
	char *rp = font_data + font_ptrs[c];
	w = *(rp++);
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

TPT_INLINE int Graphics::addchar(int x, int y, int c, int r, int g, int b, int a)
{
	int i, j, w, bn = 0, ba = 0;
	char *rp = font_data + font_ptrs[c];
	w = *(rp++);
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

TPT_INLINE void Graphics::xor_pixel(int x, int y)
{
	//OpenGL doesn't support single pixel manipulation, there are ways around it, but with poor performance
}

TPT_INLINE void Graphics::blendpixel(int x, int y, int r, int g, int b, int a)
{
	//OpenGL doesn't support single pixel manipulation, there are ways around it, but with poor performance
}

TPT_INLINE void Graphics::addpixel(int x, int y, int r, int g, int b, int a)
{
	//OpenGL doesn't support single pixel manipulation, there are ways around it, but with poor performance
}

void Graphics::xor_line(int x, int y, int x2, int y2)
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

void Graphics::xor_rect(int x, int y, int width, int height)
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

void Graphics::xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h)
{
	//glEnable(GL_COLOR_LOGIC_OP);
	//glLogicOp(GL_XOR);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textTexture);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

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
}

void Graphics::draw_line(int x, int y, int x2, int y2, int r, int g, int b, int a)
{
	a = 255;
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x2, y2);
	glEnd();
}

void Graphics::drawrect(int x, int y, int width, int height, int r, int g, int b, int a)
{
	x++;
	height--;
	width--;
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_STRIP);
	glVertex2f(x, y);
	glVertex2f(x+width, y);
	glVertex2f(x+width, y+height);
	glVertex2f(x, y+height+1); //+1 is a hack to prevent squares from missing their corners, will make smoothed lines look like SHIT
	glVertex2f(x, y);
	glEnd();
}

void Graphics::fillrect(int x, int y, int width, int height, int r, int g, int b, int a)
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

void Graphics::gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2)
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

void Graphics::clearrect(int x, int y, int width, int height)
{
	glColor4ub(0, 0, 0, 255);
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x+width, y);
	glVertex2i(x+width, y+height);
	glVertex2i(x, y+height);
	glEnd();
}

void Graphics::draw_image(pixel *img, int x, int y, int w, int h, int a)
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

#endif
