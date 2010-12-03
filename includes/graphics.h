#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "defines.h"
#include "hmap.h"

extern unsigned frame_idx;
extern unsigned cmode;

extern unsigned char fire_r[YRES/CELL][XRES/CELL];
extern unsigned char fire_g[YRES/CELL][XRES/CELL];
extern unsigned char fire_b[YRES/CELL][XRES/CELL];

extern unsigned int fire_alpha[CELL*3][CELL*3];

/*
Graphics Protypes (graphics.c)
*/
void Graphics_RenderGrid();

void Graphics_RenderWalls();

void Graphics_RenderWallsBlob();

void Graphics_RenderParticles();

void Graphics_RenderSigns();

void Graphics_RenderMenu(int i, int hover);

int Graphics_RenderToolsXY(int x, int y, int b, unsigned pc);

void Graphics_RenderIcon(int x, int y, char ch, int flag);

int Graphics_RenderThumbnail(void *thumb, int size, int bzip2, int px, int py, int scl);

void Graphics_RenderCursor(int x, int y, int t, int r);

pixel *Graphics_RescaleImage(pixel *src, int sw, int sh, int *qw, int *qh, int f);

pixel *Graphics_PrerenderSave(void *save, int size, int *width, int *height);


// Text related
int Graphics_RenderText(int x, int y, const char *s, int r, int g, int b, int a);

int Graphics_RenderWrapText(int x, int y, int w, const char *s, int r, int g, int b, int a);

int Graphics_RenderMaxText(int x, int y, int w, char *s, int r, int g, int b, int a);

int GetTextWidth(char *s);

int GetTextNWidth(char *s, int n);

void GetTextNPos(char *s, int n, int w, int *cx, int *cy);

int GetTextWidthX(char *s, int w);

int GetTextPosXY(char *s, int width, int w, int h);

#endif
