#ifdef FONTEDITOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INCLUDE_FONTDATA
//#define EXTENDED_FONT 1

#include "font.h"

#define CELLW	12
#define CELLH	10

char font[256][CELLH][CELLW];
char width[256];
unsigned char flags[256];
unsigned int color[256];
signed char top[256];
signed char left[256];

void load_char(int c)
{
	unsigned char *start = font_data + font_ptrs[c];
	unsigned char alpha, red, green, blue;
	int x, y, b;

	int w = *(start++);
#ifdef EXTENDED_FONT
	unsigned char flag = *(start++);
	signed char t = (flag&0x4) ? -(flag&0x3) : flag&0x3;
	signed char l = (flag&0x20) ? -((flag>>3)&0x3) : (flag>>3)&0x3;
	flag >>= 6;
	if (flag&0x2)
	{
		alpha = *(start++);
		red = *(start++);
		green = *(start++);
		blue = *(start++);
		color[c] = (alpha << 24) | (red << 16) | (green << 8) | blue;
	}
#endif

	if (!w)
		return;

	b = 0;
	for (y = 0; y < CELLH; y++)
		for (x = 0; x < w && x < CELLW; x++)
		{
			font[c][y][x] = ((*start) >> b) & 3;
			b += 2;
			if (b >= 8)
			{
				start++;
				b = 0;
			}
		}

	width[c] = w;
#ifdef EXTENDED_FONT
	flags[c] = flag;
	top[c] = t;
	left[c] = l;
	printf("%02X: %d %d %d %d\t0x%08X\n", c, w, t, l, flag, color[c]);
#else
	printf("%02X: %d\n", c, w);
#endif
}

char *tag = "(c) 2011 Stanislaw Skowronek";

int main(int argc, char *argv[])
{
	FILE *f;
	int i;

	memset(color, 0, sizeof(color));
	for (i = 0; i < 256; i++)
		load_char(i);

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

	return 0;
}

#endif
