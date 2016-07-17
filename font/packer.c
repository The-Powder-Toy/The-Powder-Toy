#ifdef FONTEDITOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CELLW	12
#define CELLH	10
//#define EXTENDED_FONT 1

char font[256][CELLH][CELLW];
char width[256];
unsigned char flags[256];
unsigned int color[256];
signed char top[256];
signed char left[256];

int bits_n = 0, bits_a = 0;
int flush_bits(void)
{
	if (bits_n)
	{
		bits_a >>= 8-bits_n;
		printf("0x%02X, ", bits_a);
		bits_a = 0;
		bits_n = 0;
		return 1;
	}
	return 0;
}
int stock_bits(int b, int nb)
{
	bits_a >>= nb;
	bits_a |= b << (8-nb);
	bits_n += nb;
	if (bits_n >= 8)
	{
		printf("0x%02X, ", bits_a);
		bits_a = 0;
		bits_n = 0;
		return 1;
	}
	return 0;
}

int save_char(int c)
{
	int nb = 1;
	int x, y;

	if (!width[c])
		return 0;

	printf("    0x%02hhX, ", width[c]);
#ifdef EXTENDED_FONT
	nb += stock_bits(abs(top[c])&3, 2);
	nb += stock_bits(top[c] < 0 ? 1 : 0, 1);
	nb += stock_bits(abs(left[c])&3, 2);
	nb += stock_bits(left[c] < 0 ? 1 : 0, 1);
	nb += stock_bits(flags[c]&3, 2);
	if (flags[c]&0x2)
	{
		nb += 4;
		printf("0x%02X, ", color[c]>>24);
		printf("0x%02X, ", (color[c]>>16)&0xFF);
		printf("0x%02X, ", (color[c]>>8)&0xFF);
		printf("0x%02X, ", color[c]&0xFF);
	}
#endif
	printf("  ");

	for (y = 0; y < CELLH; y++)
		for (x = 0; x < width[c] && x < CELLW; x++)
			nb += stock_bits(font[c][y][x]&3, 2);
	nb += flush_bits();

	printf("\n");

	return nb;
}

char *tag = "(c) 2008 Stanislaw Skowronek";

int main(int argc, char *argv[])
{
	FILE *f;
	int c, p[256], n = 0;

	memset(p, 0, 256*4);

	f = fopen("font.bin", "rb");
	fread(width, 1, 256, f);
#ifdef EXTENDED_FONT
	fread(flags, 1, 256, f);
	fread(color, 4, 256, f);
	fread(top, 1, 256, f);
	fread(left, 1, 256, f);
#endif
	fread(font, CELLW*CELLH, 256, f);
	fclose(f);

	printf("unsigned char font_data[] = {\n");
	for (c = 0; c < 256; c++)
	{
		p[c] = n;
		n += save_char(c);
	}
	printf("};\n");

	printf("\nshort font_ptrs[] = {\n");
	for (c = 0; c < 256; c++)
	{
		if( !(c%8))
			printf("    ");
		printf("0x%04X,", p[c]);
		if ((c%8)==7)
			printf("\n");
		else
			printf(" ");
	}
	printf("};\n");

	return 0;
}

#endif
