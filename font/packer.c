#ifdef FONTEDITOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

char *tag = "(c) 2008 Stanislaw Skowronek";

int main(int argc, char *argv[])
{
    FILE *f;
    int c, p[256], n = 0;

    memset(p, 0, 256*4);

    f = fopen("font.bin", "r");
    fread(&xsize, 1, 1, f);
    fread(&ysize, 1, 1, f);
    fread(&base, 1, 1, f);
    fread(&top, 1, 1, f);
    fread(width, 1, 256, f);
    fread(font, CELLW*CELLH, 256, f);
    fclose(f);

    printf("#define FONT_H %d\n", ysize);

    printf("char font_data[] = {\n");
    for(c=0; c<256; c++) {
	p[c] = n;
	n += save_char(c);
    }
    printf("};\n");

    printf("short font_ptrs[] = {\n");
    for(c=0; c<256; c++) {
	if(!(c%8))
	    printf("    ");
	printf("0x%04X,", p[c]);
	if((c%8)==7)
	    printf("\n");
	else
	    printf(" ");
    }
    printf("};\n");

    return 0;
}

#endif
