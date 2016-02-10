#ifdef FONTEDITOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INCLUDE_FONTDATA

#include "font.h"

#define CELLW	12
#define CELLH	10

char xsize=CELLW, ysize=CELLH;
char base=7, top=2;
char font[256][CELLH][CELLW];
char width[256];

int bits_n = 0, bits_a = 0;
int flush_bits(void)
{
    if(bits_n) {
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
    if(bits_n >= 8) {
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

    if(!width[c])
	return 0;

    printf("    0x%02X, ", width[c]);

    for(y=0; y<CELLH; y++)
	for(x=0; x<width[c]; x++)
	    nb += stock_bits(font[c][y][x]&3, 2);
    nb += flush_bits();

    printf("\n");

    return nb;
}

void load_char(int c)
{
    unsigned char *start = font_data + font_ptrs[c];
    int x, y, w, b;

    w = *(start ++);

    if(!w)
	return;

    b = 0;
    for(y=0; y<CELLH; y++)
	for(x=0; x<w; x++) {
	    font[c][y][x] = ((*start) >> b) & 3;
	    b += 2;
	    if(b >= 8) {
		start ++;
		b = 0;
	    }
	}

    width[c] = w;
printf("%02x: %d\n", c, w);
}

char *tag = "(c) 2011 Stanislaw Skowronek";

int main(int argc, char *argv[])
{
    FILE *f;
    int i;

    for(i=0; i<sizeof(font_ptrs)/sizeof(short); i++)
	load_char(i);

    f = fopen("font.bin", "w");
    fwrite(&xsize, 1, 1, f);
    fwrite(&ysize, 1, 1, f);
    fwrite(&base, 1, 1, f);
    fwrite(&top, 1, 1, f);
    fwrite(width, 1, 256, f);
    fwrite(font, CELLW*CELLH, 256, f);
    fclose(f);

    return 0;
}

#endif
