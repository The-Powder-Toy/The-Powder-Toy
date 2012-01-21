#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL/SDL.h>
#include <string>
#include "Config.h"
//#include "powder.h"

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
#elif defined(PIX32OGL)
#define PIXPACK(x) (0xFF000000|((x)&0xFFFFFF))
#define PIXRGB(r,g,b) (0xFF000000|((r)<<16)|((g)<<8)|((b)))// (((b)<<16)|((g)<<8)|(r))
#define PIXRGBA(r,g,b,a) (((a)<<24)|((r)<<16)|((g)<<8)|((b)))// (((b)<<16)|((g)<<8)|(r))
#define PIXA(x) (((x)>>24)&0xFF)
#define PIXR(x) (((x)>>16)&0xFF)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)&0xFF)
#else
#define PIXPACK(x) (x)
#define PIXRGB(r,g,b) (((r)<<16)|((g)<<8)|(b))
#define PIXR(x) ((x)>>16)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)&0xFF)
#endif
#endif
#endif

#ifdef PIX16
typedef unsigned short pixel;
#else
typedef unsigned int pixel;
#endif

/*extern int emp_decor;

extern unsigned int *render_modes;
extern unsigned int render_mode;
extern unsigned int colour_mode;
extern unsigned int *display_modes;
extern unsigned int display_mode;

extern SDL_Surface *sdl_scrn;
extern int sdl_scale;

extern int sandcolour_r;
extern int sandcolour_g;
extern int sandcolour_b;
extern int sandcolour_frame;

extern unsigned char fire_r[YRES/CELL][XRES/CELL];
extern unsigned char fire_g[YRES/CELL][XRES/CELL];
extern unsigned char fire_b[YRES/CELL][XRES/CELL];

extern unsigned int fire_alpha[CELL*3][CELL*3];
extern pixel *pers_bg;

extern char * flm_data;
extern int flm_data_points;
extern pixel flm_data_colours[];
extern float flm_data_pos[];

extern char * plasma_data;
extern int plasma_data_points;
extern pixel plasma_data_colours[];
extern float plasma_data_pos[];*/

class Graphics
{
public:
	SDL_Surface * sdl_scrn;
	pixel *vid;
	pixel *render_packed_rgb(void *image, int width, int height, int cmp_size);
	static char * generate_gradient(pixel * colours, float * points, int pointcount, int size);
	void draw_other();
	void draw_rgba_image(unsigned char *data, int x, int y, float a);
	static void *ptif_pack(pixel *src, int w, int h, int *result_size);
	static pixel *ptif_unpack(void *datain, int size, int *w, int *h);
	static pixel *resample_img_nn(pixel *src, int sw, int sh, int rw, int rh);
	static pixel *resample_img(pixel *src, int sw, int sh, int rw, int rh);
	static pixel *rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f);
	//void render_gravlensing(pixel *src, pixel * dst);
	//void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch);
	//void sdl_blit_2(int x, int y, int w, int h, pixel *src, int pitch);
	//void sdl_blit(int x, int y, int w, int h, pixel *src, int pitch);
	void drawblob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);
	void draw_tool(int b, int sl, int sr, unsigned pc, unsigned iswall);
	//int draw_tool_xy(pixel *vid_buf, int x, int y, int b, unsigned pc);
	//void draw_menu(pixel *vid_buf, int i, int hover);
	void drawpixel(int x, int y, int r, int g, int b, int a);
	int addchar(int x, int y, int c, int r, int g, int b, int a);
	int drawchar(int x, int y, int c, int r, int g, int b, int a);
	int drawtext(int x, int y, std::string &s, int r, int g, int b, int a);
	int drawtext(int x, int y, const char *s, int r, int g, int b, int a);
	int drawtext_outline(int x, int y, const char *s, int r, int g, int b, int a, int olr, int olg, int olb, int ola);
	int drawtextwrap(int x, int y, int w, const char *s, int r, int g, int b, int a);
	void drawrect(int x, int y, int w, int h, int r, int g, int b, int a);
	void fillrect(int x, int y, int w, int h, int r, int g, int b, int a);
	void clearrect(int x, int y, int w, int h);
	void drawdots(int x, int y, int h, int r, int g, int b, int a);
	static int textwidth(char *s);
	int drawtextmax(int x, int y, int w, char *s, int r, int g, int b, int a);
	static int textnwidth(char *s, int n);
	static void textnpos(char *s, int n, int w, int *cx, int *cy);
	static int textwidthx(char *s, int w);
	static int textposxy(char *s, int width, int w, int h);
	static int textwrapheight(char *s, int width);
	void blendpixel(int x, int y, int r, int g, int b, int a);
	void draw_icon(int x, int y, char ch, int flag);
	//void draw_air();
	//void draw_grav_zones(pixel *vid);
	//void draw_grav(pixel *vid);
	void draw_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
	void addpixel(int x, int y, int r, int g, int b, int a);
	void xor_pixel(int x, int y);
	void xor_line(int x1, int y1, int x2, int y2);
	void xor_rect(int x, int y, int w, int h);
	void blend_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
	//void render_parts(pixel *vid);
//	#ifdef OGLR
//	void draw_parts_fbo();
//	#endif
//	void draw_parts();
//	void draw_walls(pixel *vid);
//	void create_decorations(int x, int y, int rx, int ry, int r, int g, int b, int click, int tool);
//	void create_decoration(int x, int y, int r, int g, int b, int click, int tool);
//	void line_decorations(int x1, int y1, int x2, int y2, int rx, int ry, int r, int g, int b, int click, int tool);
//	void box_decorations(int x1, int y1, int x2, int y2, int r, int g, int b, int click, int tool);
//	void draw_color_menu(pixel *vid_buf, int i, int hover);
	void draw_wavelengths(int x, int y, int h, int wl);
	void render_signs();
//	void render_fire(pixel *dst);
//	void prepare_alpha(int size, float intensity);
	void draw_image(pixel *img, int x, int y, int w, int h, int a);
	static void dim_copy(pixel *dst, pixel *src);
	static void dim_copy_pers(pixel *dst, pixel *src);
	//void render_zoom(pixel *img);
	//int render_thumb(void *thumb, int size, int bzip2, pixel *vid_buf, int px, int py, int scl);
	//void render_cursor(pixel *vid, int x, int y, int t, int rx, int ry);
	//int sdl_open(void);
	//int draw_debug_info(pixel* vid, int lm, int lx, int ly, int cx, int cy, int line_x, int line_y);
	void Clear();
	void Blit();
	void AttachSDLSurface(SDL_Surface * surface);
	#ifdef OGLR
	void clearScreen(float alpha);
	void ogl_blit(int x, int y, int w, int h, pixel *src, int pitch, int scale);
	#endif
	Graphics();
	~Graphics();
};

#endif
