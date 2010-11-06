#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL/SDL.h>
#include "defines.h"
#include "hmap.h"

#ifdef PIX16
#define PIXELSIZE 2
#define PIXPACK(x) ((((x)>>8)&0xF800)|(((x)>>5)&0x07E0)|(((x)>>3)&0x001F))
#define PIXRGB(r,g,b) ((((r)<<8)&0xF800)|(((g)<<3)&0x07E0)|(((b)>>3)&0x001F))
#define PIXR(x) (((x)>>8)&0xF8)
#define PIXG(x) (((x)>>3)&0xFC)
#define PIXB(x) (((x)<<3)&0xF8)
#else
#define PIXELSIZE 4
#ifdef PIX32BGR
#define PIXPACK(x) ((((x)>>16)&0x0000FF)|((x)&0x00FF00)|(((x)<<16)&0xFF0000))
#define PIXRGB(r,g,b) (((b)<<16)|((g)<<8)|((r)))// (((b)<<16)|((g)<<8)|(r))
#define PIXR(x) ((x)&0xFF)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)>>16)
#else
#ifdef PIX32BGRA
#define PIXPACK(x) ((((x)>>8)&0x0000FF00)|(((x)<<8)&0x00FF0000)|(((x)<<24)&0xFF000000))
#define PIXRGB(r,g,b) (((b)<<24)|((g)<<16)|((r)<<8))
#define PIXR(x) (((x)>>8)&0xFF)
#define PIXG(x) (((x)>>16)&0xFF)
#define PIXB(x) (((x)>>24))
#else
#define PIXPACK(x) (x)
#define PIXRGB(r,g,b) (((r)<<16)|((g)<<8)|(b))
#define PIXR(x) ((x)>>16)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)&0xFF)
#endif
#endif
#endif

extern unsigned cmode;
extern SDL_Surface *sdl_scrn;
extern int sdl_scale;

extern unsigned char fire_r[YRES/CELL][XRES/CELL];
extern unsigned char fire_g[YRES/CELL][XRES/CELL];
extern unsigned char fire_b[YRES/CELL][XRES/CELL];

extern unsigned int fire_alpha[CELL*3][CELL*3];
extern pixel *fire_bg;

pixel *rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f);

void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch);

void sdl_blit_2(int x, int y, int w, int h, pixel *src, int pitch);

void sdl_blit(int x, int y, int w, int h, pixel *src, int pitch);

void drawblob(pixel *vid, int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);

void draw_tool(pixel *vid_buf, int b, int sl, int sr, unsigned pc, unsigned iswall);

int draw_tool_xy(pixel *vid_buf, int x, int y, int b, unsigned pc);

void draw_menu(pixel *vid_buf, int i, int hover);

#if defined(WIN32) && !defined(__GNUC__)
_inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a);
#else
extern inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a);
#endif

#if defined(WIN32) && !defined(__GNUC__)
_inline int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a);
#else
extern inline int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a);
#endif

int drawtext(pixel *vid, int x, int y, const char *s, int r, int g, int b, int a);

int drawtextwrap(pixel *vid, int x, int y, int w, const char *s, int r, int g, int b, int a);

void drawrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a);

void fillrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a);

void clearrect(pixel *vid, int x, int y, int w, int h);

void drawdots(pixel *vid, int x, int y, int h, int r, int g, int b, int a);

int textwidth(char *s);

int drawtextmax(pixel *vid, int x, int y, int w, char *s, int r, int g, int b, int a);

int textnwidth(char *s, int n);

void textnpos(char *s, int n, int w, int *cx, int *cy);

int textwidthx(char *s, int w);

int textposxy(char *s, int width, int w, int h);

#if defined(WIN32) && !defined(__GNUC__)
_inline void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a);
#else
void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a);
#endif

void draw_icon(pixel *vid_buf, int x, int y, char ch, int flag);

void draw_air(pixel *vid);

void draw_line(pixel *vid, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

void addpixel(pixel *vid, int x, int y, int r, int g, int b, int a);

void xor_pixel(int x, int y, pixel *vid);

void xor_line(int x1, int y1, int x2, int y2, pixel *vid);

void xor_rect(pixel *vid, int x, int y, int w, int h);

void draw_parts(pixel *vid);

void render_signs(pixel *vid_buf);

void render_fire(pixel *dst);

void prepare_alpha(void);

void draw_image(pixel *vid, pixel *img, int x, int y, int w, int h, int a);

void dim_copy(pixel *dst, pixel *src);

void render_zoom(pixel *img);

pixel *prerender_save(void *save, int size, int *width, int *height);

int render_thumb(void *thumb, int size, int bzip2, pixel *vid_buf, int px, int py, int scl);

void render_cursor(pixel *vid, int x, int y, int t, int r);

void sdl_open(void);

#ifdef OpenGL
void Enable2D ();
void RenderScene ();
void ClearScreen();
#endif

#endif
