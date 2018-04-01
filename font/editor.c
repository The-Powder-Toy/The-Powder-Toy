#ifdef FONTEDITOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>

#define CELLW	12
#define CELLH	10

#define XRES	800
#define YRES	600
//#define SCALE	1
//#define EXTENDED_FONT 1

char font[256][CELLH][CELLW];
char width[256];
unsigned char flags[256];
unsigned int color[256];
signed char top[256];
signed char left[256];

void blendpixel(unsigned *vid, int x, int y, int r, int g, int b, int a)
{
	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return;
	if (a != 255)
	{
		int t = vid[y*XRES+x];
		r = (a*r + (255-a)*((t>>16)&255)) >> 8;
		g = (a*g + (255-a)*((t>>8)&255)) >> 8;
		b = (a*b + (255-a)*(t&255)) >> 8;
	}
	vid[y*XRES+x] = (r<<16)|(g<<8)|b;
}

int drawchar(unsigned *vid, int x, int y, unsigned char c, int r, int g, int b)
{
	int i, j;
	if (color[c])
	{
		r = (color[c] >> 16) & 0xFF;
		g = (color[c] >> 8) & 0xFF;
		b = color[c] & 0xFF;
	}
	for (j = 0; j < CELLH; j++)
		for (i = 0; i < width[c] && i < CELLW; i++)
			blendpixel(vid, x+i+left[c], y+j+top[c], r, g, b, (font[c][j][i]*255)/3);
	return x + width[c];
}

int drawtext(unsigned *vid, int x, int y, const char *s, int r, int g, int b)
{
	for (; *s; s++)
		x = drawchar(vid, x, y, *s, r, g, b);
	return x;
}

void drawline(unsigned *vid, int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	int cp = abs(y2-y1) > abs(x2-x1), x, y, dx, dy, sy;
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
	de = dy/(float)dx;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (cp)
			blendpixel(vid, y, x, r, g, b, a);
		else
			blendpixel(vid, x, y, r, g, b, a);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

void drawcell(unsigned *vid, int i, int j, int c, int m)
{
	int x, y, x0=i*32+64, y0=j*32+64;
	for (y = 1;y < 32;y++)
		for (x = 1; x < 32; x++)
			blendpixel(vid, x0+x, y0+y, 127*m, 127*m, 127*m, c);
	for (x = 0; x <32; x+=2)
	{
		if (!j)
			blendpixel(vid, x0+x, y0, 64*m, 64*m, 64*m, 255);
		if (!i)
			blendpixel(vid, x0, y0+x, 64*m, 64*m, 64*m, 255);
		blendpixel(vid, x0+x, y0+32, 64*m, 64*m, 64*m, 255);
		blendpixel(vid, x0+32, y0+x, 64*m, 64*m, 64*m, 255);
	}
}

/***********************************************************
 *                       SDL OUTPUT                        *
 ***********************************************************/

SDL_Surface *sdl_scrn;
int sdl_key;
void sdl_open(void)
{
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	sdl_scrn = SDL_SetVideoMode(XRES*SCALE, YRES*SCALE + 40*SCALE, 32, SDL_SWSURFACE);
	if (!sdl_scrn)
	{
		fprintf(stderr, "Creating window: %s\n", SDL_GetError());
		exit(1);
	}
}

void sdl_blit(int x, int y, int w, int h, unsigned int *src, int pitch)
{
	unsigned *dst,i,j,k;
	if (SDL_MUSTLOCK(sdl_scrn))
		if (SDL_LockSurface(sdl_scrn)<0)
			return;
	dst = (unsigned *)sdl_scrn->pixels+y*sdl_scrn->pitch/4+x;
	for (j = 0;j < h; j++)
	{
		for (k = 0; k < SCALE; k++)
	{
			for (i = 0; i < w*SCALE; i++)
				dst[i] = src[i/SCALE];
			dst += sdl_scrn->pitch/4;
		}
		src += pitch/4;
	}
	if (SDL_MUSTLOCK(sdl_scrn))
		SDL_UnlockSurface(sdl_scrn);
	SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

int frame_idx = 0;
void dump_frame(unsigned int *src, int w, int h, int pitch)
{
	char frame_name[32];
	unsigned j,i,c;
	FILE *f;
	sprintf(frame_name,"frame%04d.ppm",frame_idx);
	f=fopen(frame_name,"w");
	fprintf(f,"P6\n%d %d\n255\n",w,h);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			c = ((src[i]&0xFF0000)>>16) | (src[i]&0x00FF00) | ((src[i]&0x0000FF)<<16);
			fwrite(&c,3,1,f);
		}
		src += pitch/4;
	}
	fclose(f);
	frame_idx++;
}

int sdl_poll()
{
	SDL_Event event;
	sdl_key=0;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			sdl_key = event.key.keysym.sym;
			break;
		case SDL_QUIT:
			return 1;
		}
	}
	return 0;
}

/***********************************************************
 *                      MAIN PROGRAM                       *
 ***********************************************************/

const char *tag = "(c) 2008 Stanislaw Skowronek";

int main(int argc, char *argv[])
{
	unsigned *vid_buf = (unsigned*)calloc(XRES*YRES, sizeof(unsigned));
	int x, y, b = 0, lb, c = 0xA0, i, j, dc = 0;
	int mode = 0;
	char hex[18] = "";
	char inputColor = 0;
	FILE *f;

	f = fopen("font.bin", "rb");
	if(f)
	{
		fread(width, 1, 256, f);
#ifdef EXTENDED_FONT
		fread(flags, 1, 256, f);
		fread(color, 4, 256, f);
		fread(top, 1, 256, f);
		fread(left, 1, 256, f);
#endif
		fread(font, CELLW*CELLH, 256, f);
		fclose(f);
	}

	sdl_open();
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	while (!sdl_poll())
	{
		if (sdl_key=='q' || sdl_key==SDLK_ESCAPE)
			break;
		else if ((sdl_key==' ' || sdl_key=='=') && c < 255)
		{
			if (inputColor)
			{
				inputColor = 0;
				color[c] = 0;
				flags[c] &= ~0x2;
			}
			c++;
		}
		else if ((sdl_key=='\b' || sdl_key=='-') && c > 0)
		{
			if (inputColor)
			{
				inputColor = 0;
				color[c] = 0;
				flags[c] &= ~0x2;
			}
			c--;
		}
#ifdef EXTENDED_FONT
		else if (sdl_key == 'w')
			flags[c] ^= 0x1;
		else if (sdl_key == 'c')
		{
			if (!(flags[c] & 0x2))
			{
				flags[c] |= 0x2;
				color[c] = 255<<24;
				inputColor = 1;
			}
			else
			{
				flags[c] &= ~0x2;
				color[c] = 0;
				inputColor = 0;
			}
		}
		else if (sdl_key == SDLK_RETURN)
		{
			int val = (x > 255) ? 255 : x;
			switch (inputColor)
			{
				case 1:
					color[c] |= (val<<16);
					inputColor = 2;
					break;
				case 2:
					color[c] |= (val<<8);
					inputColor = 3;
					break;
				case 3:
					color[c] |= (val<<0);
					inputColor = 0;
					break;
			}
		}
#endif

		lb = b;
		b = SDL_GetMouseState(&x, &y);
		if (b)
		{
			x /= SCALE;
			y /= SCALE;
		}

		i = x/32-2;
		j = y/32-2;
		if (!lb && b && (j<0 || j>=CELLH || i<0 || i>=CELLW))
		{
			int leftDistance = abs(i-left[c]);
			int widthDistance = abs(i-width[c]);
			int topDistance = abs(j-top[c]);
			if (widthDistance <= leftDistance && widthDistance <= topDistance)
				mode = 3;	// WIDTH
			else if (leftDistance <= topDistance)
				mode = 4;	// LEFT
			else
				mode = 2;	// TOP
		}
		else if (!lb && b)
		{
			mode = 1;		// DRAW
			if (b == 1)
				dc = (font[c][j][i]+3)%4;
			else
				dc = (font[c][j][i]+1)%4;
		}

		if (b)
			switch(mode)
			{
			case 1:
				if (i>=0 && j>=0 && i<CELLW && j<CELLH)
					font[c][j][i] = dc;
				break;
#ifdef EXTENDED_FONT
			case 2:
				top[c] = j;
				break;
#endif
			case 3:
				width[c] = i;
				break;
#ifdef EXTENDED_FONT
			case 4:
				left[c] = i;
				break;
#endif
			}

		memset(vid_buf, 0, XRES*YRES*4);
		for (j = 0; j < CELLH; j++)
			for (i = 0; i < CELLW; i++)
				drawcell(vid_buf, i, j, (font[c][j][i]*255)/3, i>=width[c]?1:2);
		drawline(vid_buf, 32, 64+top[c]*32, 128+32*CELLW, 64+top[c]*32, 128, 128, 255, 255);
		drawline(vid_buf, 64+width[c]*32, 32, 64+width[c]*32, 128+32*CELLH, 255, 128, 128, 255);
		drawline(vid_buf, 64+left[c]*32, 32, 64+left[c]*32, 128+32*CELLH, 128, 255, 128, 255);

		drawtext(vid_buf, 64, 192+32*CELLH, "A quick brown fox jumps over the lazy dog.", 255, 255, 255);
		drawtext(vid_buf, 64, 192+33*CELLH, "A QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", 255, 255, 255);
		drawtext(vid_buf, 64, 192+34*CELLH, "0123456789 ~`!@#$%^&*()-=_+[]{}\\|;:'\",./<>?", 255, 255, 255);

		drawtext(vid_buf, 64, 192+37*CELLH, "Use '+' (= key) and '-' to switch between characters", 255, 255, 255);
#ifdef EXTENDED_FONT
		drawtext(vid_buf, 64, 192+38*CELLH, "Click near a line to modify top/left offset, or character width", 255, 255, 255);
		drawtext(vid_buf, 64, 192+39*CELLH, "Use 'w' to toggle ignore width & auto draw next character flag, and 'c' to add / turn off font color", 255, 255, 255);
		if (inputColor)
		{
			char temptext[64];
			//drawtext(vid_buf, 64, 192+40*CELLH, "Due to extreme laziness, you must move the mouse and press enter to set the color of this icon", 255, 255, 255);
			sprintf(temptext, "Press enter to set %s color to: %i", inputColor == 1 ? "red" : (inputColor == 2 ? "green" : "blue"), (x > 255) ? 255 : x);
			drawtext(vid_buf, 64, 192+40*CELLH, temptext, 255, 255, 255);
		}
#endif

		drawchar(vid_buf, 32, 192+32*CELLH, c, 255, 255, 255);
		
		sprintf(hex, "%02X", c);
		drawtext(vid_buf, 32, 192+34*CELLH, hex, 255, 255, 255);
#ifdef EXTENDED_FONT
		sprintf(hex, "flags: 0x%02X", flags[c]);
		drawtext(vid_buf, 32, 192+35*CELLH, hex, 255, 255, 255);
		sprintf(hex, "color: 0x%08X", color[c]);
		drawtext(vid_buf, 32, 192+36*CELLH, hex, 255, 255, 255);
#endif

		sdl_blit(0, 0, XRES, YRES, vid_buf, XRES*4);
	}

	f = fopen("font.bin", "wb");
	fwrite(width, 1, 256, f);
#ifdef EXTENDED_FONT
	fwrite(flags, 1, 256, f);
	fwrite(color, 4, 256, f);
	fwrite(top, 1, 256, f);
	fwrite(left, 1, 256, f);
#endif
	fwrite(font, CELLW*CELLH, 256, f);
	fclose(f);
	free(vid_buf);

	return 0;
}

#endif
