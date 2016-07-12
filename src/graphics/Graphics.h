#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#if defined(OGLI)
#include "OpenGLHeaders.h"
#endif
#include "Config.h"
//#include "powder.h"

#define PIXELCHANNELS 3
#ifdef PIX16
#define PIXELSIZE 2
#define PIXPACK(x) ((((x)>>8)&0xF800)|(((x)>>5)&0x07E0)|(((x)>>3)&0x001F))					//16bit RGB in 16bit int: ????
#define PIXRGB(r,g,b) ((((r)<<8)&0xF800)|(((g)<<3)&0x07E0)|(((b)>>3)&0x001F))
#define PIXR(x) (((x)>>8)&0xF8)
#define PIXG(x) (((x)>>3)&0xFC)
#define PIXB(x) (((x)<<3)&0xF8)
#else
#define PIXELSIZE 4
#ifdef PIX32BGRA
#define PIXPACK(x) ((((x)>>16)&0x0000FF)|((x)&0x00FF00)|(((x)<<16)&0xFF0000))				//24bit BGR in 32bit int: 00BBGGRR
#define PIXRGB(r,g,b) (((b)<<16)|((g)<<8)|((r)))// (((b)<<16)|((g)<<8)|(r))
#define PIXR(x) ((x)&0xFF)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)>>16)
#else
#ifdef PIX32BGRA
#define PIXPACK(x) ((((x)>>8)&0x0000FF00)|(((x)<<8)&0x00FF0000)|(((x)<<24)&0xFF000000))		//32bit BGRA in 32bit int: BBGGRRAA
#define PIXRGB(r,g,b) (((b)<<24)|((g)<<16)|((r)<<8))
#define PIXR(x) (((x)>>8)&0xFF)
#define PIXG(x) (((x)>>16)&0xFF)
#define PIXB(x) (((x)>>24)&0xFF)
#elif defined(PIX32OGL)
#undef PIXELCHANNELS
#define PIXELCHANNELS 4
#define PIXPACK(x) (0xFF000000|((x)&0xFFFFFF))												//32bit ARGB in 32bit int: AARRGGBB
#define PIXRGB(r,g,b) (0xFF000000|((r)<<16)|((g)<<8)|((b)))
#define PIXRGBA(r,g,b,a) (((a)<<24)|((r)<<16)|((g)<<8)|((b)))
#define PIXA(x) (((x)>>24)&0xFF)
#define PIXR(x) (((x)>>16)&0xFF)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)&0xFF)
#else
#define PIXPACK(x) (x)																		//24bit RGB in 32bit int: 00RRGGBB.
#define PIXRGB(r,g,b) (((r)<<16)|((g)<<8)|(b))
#define PIXR(x) (((x)>>16)&0xFF)
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

//Icon names, see Graphics::draw_icon
enum Icon
{
	NoIcon = 0,
	IconOpen,
	IconReload,
	IconSave,
	IconVoteUp,
	IconVoteDown,
	IconTag,
	IconNew,
	IconLogin,
	IconRenderSettings,
	IconSimulationSettings,
	IconPause,
	IconVoteSort,
	IconDateSort,
	IconMyOwn,
	IconFavourite,
	IconSearch,
	IconDelete,
	IconAdd,
	IconReport,
	IconUsername,
	IconPassword,
	IconClose,
	IconEffect,
	IconFire,
	IconGlow,
	IconBlur,
	IconBlob,
	IconBasic,
	IconAltAir,
	IconPressure,
	IconVelocity,
	IconWarp,
	IconPersistant,
	IconHeat,
	IconLife,
	IconGradient
};

//"Graphics lite" - slightly lower performance due to variable size,
class VideoBuffer
{
public:
	pixel * Buffer;
	int Width, Height;

	VideoBuffer(const VideoBuffer & old);
	VideoBuffer(VideoBuffer * old);
	VideoBuffer(pixel * buffer, int width, int height);
	VideoBuffer(int width, int height);
	void Resize(float factor, bool resample = false);
	void Resize(int width, int height, bool resample = false, bool fixedRatio = true);
	TPT_INLINE void BlendPixel(int x, int y, int r, int g, int b, int a)
	{
	#ifdef PIX32OGL
		pixel t;
		if (x<0 || y<0 || x>=Width || y>=Height)
			return;
		if (a!=255)
		{
			t = Buffer[y*(Width)+x];
			r = (a*r + (255-a)*PIXR(t)) >> 8;
			g = (a*g + (255-a)*PIXG(t)) >> 8;
			b = (a*b + (255-a)*PIXB(t)) >> 8;
			a = a > PIXA(t) ? a : PIXA(t);
		}
		Buffer[y*(Width)+x] = PIXRGBA(r,g,b,a);
	#else
		pixel t;
		if (x<0 || y<0 || x>=Width || y>=Height)
			return;
		if (a!=255)
		{
			t = Buffer[y*(Width)+x];
			r = (a*r + (255-a)*PIXR(t)) >> 8;
			g = (a*g + (255-a)*PIXG(t)) >> 8;
			b = (a*b + (255-a)*PIXB(t)) >> 8;
		}
		Buffer[y*(Width)+x] = PIXRGB(r,g,b);
	#endif
	}

	TPT_INLINE void SetPixel(int x, int y, int r, int g, int b, int a)
	{
		if (x<0 || y<0 || x>=Width || y>=Height)
				return;
	#ifdef PIX32OGL
		Buffer[y*(Width)+x] = PIXRGBA(r,g,b,a);
	#else
		Buffer[y*(Width)+x] = PIXRGB((r*a)>>8, (g*a)>>8, (b*a)>>8);
	#endif
	}

	TPT_INLINE void AddPixel(int x, int y, int r, int g, int b, int a)
	{
		pixel t;
		if (x<0 || y<0 || x>=Width || y>=Height)
			return;
		t = Buffer[y*(Width)+x];
		r = (a*r + 255*PIXR(t)) >> 8;
		g = (a*g + 255*PIXG(t)) >> 8;
		b = (a*b + 255*PIXB(t)) >> 8;
		if (r>255)
			r = 255;
		if (g>255)
			g = 255;
		if (b>255)
			b = 255;
		Buffer[y*(Width)+x] = PIXRGB(r,g,b);
	}
	int SetCharacter(int x, int y, int c, int r, int g, int b, int a);
	int BlendCharacter(int x, int y, int c, int r, int g, int b, int a);
	int AddCharacter(int x, int y, int c, int r, int g, int b, int a);
	~VideoBuffer();
};

class Graphics
{
public:
	pixel *vid;
	int sdl_scale;
#ifdef OGLI
	//OpenGL specific instance variables
	GLuint vidBuf, textTexture;
	void Reset();
	void LoadDefaults();
	void InitialiseTextures();
	void DestroyTextures();
 #endif

	//Common graphics methods in Graphics.cpp
	static char * GenerateGradient(pixel * colours, float * points, int pointcount, int size);

	//PTIF methods
	static void *ptif_pack(pixel *src, int w, int h, int *result_size);
	static pixel *ptif_unpack(void *datain, int size, int *w, int *h);
	static pixel *resample_img_nn(pixel *src, int sw, int sh, int rw, int rh);
	static pixel *resample_img(pixel *src, int sw, int sh, int rw, int rh);
	static pixel *rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f);
	static pixel *render_packed_rgb(void *image, int width, int height, int cmp_size);

	//Font/text metrics
	static int CharIndexAtPosition(char *s, int positionX, int positionY);
	static int PositionAtCharIndex(char *s, int charIndex, int & positionX, int & positionY);
	static int CharWidth(unsigned char c);
	static int textnwidth(char *s, int n);
	static void textnpos(char *s, int n, int w, int *cx, int *cy);
	static int textwidthx(char *s, int w);
	static int textposxy(char *s, int width, int w, int h);
	static int textwrapheight(char *s, int width);
	static int textwidth(const char *s);
	static void textsize(const char * s, int & width, int & height);

	VideoBuffer DumpFrame();

	void Acquire();
	void Release();

	void blendpixel(int x, int y, int r, int g, int b, int a);
	void addpixel(int x, int y, int r, int g, int b, int a);

	void draw_icon(int x, int y, Icon icon, unsigned char alpha = 255, bool invert = false);

	void Clear();
	void Finalise();
	//
	int drawtext_outline(int x, int y, const char *s, int r, int g, int b, int a);
	int drawtext(int x, int y, const char *s, int r, int g, int b, int a);
	int drawtext(int x, int y, std::string s, int r, int g, int b, int a);
	int drawchar(int x, int y, int c, int r, int g, int b, int a);
	int addchar(int x, int y, int c, int r, int g, int b, int a);

	void xor_pixel(int x, int y);
	void xor_line(int x, int y, int x2, int y2);
	void xor_rect(int x, int y, int width, int height);
	void xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h);

	void draw_line(int x, int y, int x2, int y2, int r, int g, int b, int a);
	void drawrect(int x, int y, int width, int height, int r, int g, int b, int a);
	void fillrect(int x, int y, int width, int height, int r, int g, int b, int a);
	void drawcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	void fillcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	void clearrect(int x, int y, int width, int height);
	void gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2);

	void draw_image(pixel *img, int x, int y, int w, int h, int a);
	void draw_image(const VideoBuffer & vidBuf, int w, int h, int a);
	void draw_image(VideoBuffer * vidBuf, int w, int h, int a);
	void draw_rgba_image(const unsigned char *data, int x, int y, float alpha);

	Graphics();
	~Graphics();
};

#endif
