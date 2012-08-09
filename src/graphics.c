/**
 * Powder Toy - graphics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <SDL/SDL.h>
#include <bzlib.h>
#ifdef WIN32
#include <SDL/SDL_syswm.h>
#endif

#if defined(OGLR)
#ifdef MACOSX
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#elif defined(WIN32)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#include <defines.h>
#include <air.h>
#include "gravity.h"
#include <powder.h>
#define INCLUDE_SHADERS
#include <graphics.h>
#include <powdergraphics.h>
#define INCLUDE_FONTDATA
#include <font.h>
#include <misc.h>
#include "hmap.h"
#ifdef LUACONSOLE
#include <luaconsole.h>
#endif

#if defined(LIN32) || defined(LIN64)
#include "icon.h"
#endif


#ifdef WIN32
IMAGE_DOS_HEADER __ImageBase;
#endif

//unsigned cmode = CM_FIRE;
unsigned int *render_modes;
unsigned int render_mode;
unsigned int colour_mode;
unsigned int *display_modes;
unsigned int display_mode;

SDL_Surface *sdl_scrn;
int sdl_scale = 1;

#ifdef OGLR
GLuint zoomTex, vidBuf, airBuf, fireAlpha, glowAlpha, blurAlpha, partsFboTex, partsFbo, partsTFX, partsTFY, airPV, airVY, airVX;
GLuint fireProg, airProg_Pressure, airProg_Velocity, airProg_Cracker, lensProg;
#endif

int emp_decor = 0;
int sandcolour_r = 0;
int sandcolour_g = 0;
int sandcolour_b = 0;
int sandcolour_frame = 0;

unsigned char fire_r[YRES/CELL][XRES/CELL];
unsigned char fire_g[YRES/CELL][XRES/CELL];
unsigned char fire_b[YRES/CELL][XRES/CELL];

unsigned int fire_alpha[CELL*3][CELL*3];
pixel *pers_bg;

char * flm_data;
int flm_data_points = 4;
pixel flm_data_colours[] = {PIXPACK(0xAF9F0F), PIXPACK(0xDFBF6F), PIXPACK(0x60300F), PIXPACK(0x000000)};
float flm_data_pos[] = {1.0f, 0.9f, 0.5f, 0.0f};

char * plasma_data;
int plasma_data_points = 5;
pixel plasma_data_colours[] = {PIXPACK(0xAFFFFF), PIXPACK(0xAFFFFF), PIXPACK(0x301060), PIXPACK(0x301040), PIXPACK(0x000000)};
float plasma_data_pos[] = {1.0f, 0.9f, 0.5f, 0.25, 0.0f};

void init_display_modes()
{
	int i;
	display_modes = calloc(1, sizeof(unsigned int));
	render_modes = calloc(2, sizeof(unsigned int));
	
	display_modes[0] = 0;
	render_modes[0] = RENDER_FIRE;
	render_modes[1] = 0;
	
	update_display_modes();
}

// Combine all elements of the display_modes and render_modes arrays into single variables using bitwise or
void update_display_modes()
{
	int i;
	display_mode = 0;
	i = 0;
	while(display_modes[i])
	{
		display_mode |= display_modes[i];
		i++;
	}
	render_mode = 0;
	i = 0;
	while(render_modes[i])
	{
		render_mode |= render_modes[i];
		i++;
	}
}

char * generate_gradient(pixel * colours, float * points, int pointcount, int size)
{
	int cp, i, j;
	pixel ptemp;
	char * newdata = malloc(size * 3);
	float poss, pose, temp;
	memset(newdata, 0, size*3);
	//Sort the Colours and Points
	for (i = (pointcount - 1); i > 0; i--)
	{
		for (j = 1; j <= i; j++)
		{
			if (points[j-1] > points[j])
			{
				temp = points[j-1];
				points[j-1] = points[j];
				points[j] = temp;
				
				ptemp = colours[j-1];
				colours[j-1] = colours[j];
				colours[j] = ptemp;
			}
		}
	}
	i = 0;
	j = 1;
	poss = points[i];
	pose = points[j];
	for (cp = 0; cp < size; cp++)
	{
		float cpos = (float)cp / (float)size, ccpos, cccpos;
		if(cpos > pose && j+1 < pointcount)
		{
			poss = points[++i];
			pose = points[++j];
		}
		ccpos = cpos - poss;
		cccpos = ccpos / (pose - poss);
		if(cccpos > 1.0f)
			cccpos = 1.0f;
		newdata[(cp*3)] = PIXR(colours[i])*(1.0f-cccpos) + PIXR(colours[j])*(cccpos);
		newdata[(cp*3)+1] = PIXG(colours[i])*(1.0f-cccpos) + PIXG(colours[j])*(cccpos);
		newdata[(cp*3)+2] = PIXB(colours[i])*(1.0f-cccpos) + PIXB(colours[j])*(cccpos);
	}
	return newdata;
}

void *ptif_pack(pixel *src, int w, int h, int *result_size){
	int i = 0, datalen = (w*h)*3, cx = 0, cy = 0;
	unsigned char *red_chan = calloc(1, w*h); 
	unsigned char *green_chan = calloc(1, w*h); 
	unsigned char *blue_chan = calloc(1, w*h); 
	unsigned char *data = malloc(((w*h)*3)+8);
	unsigned char *result = malloc(((w*h)*3)+8);
	
	for(cx = 0; cx<w; cx++){
		for(cy = 0; cy<h; cy++){
			red_chan[w*(cy)+(cx)] = PIXR(src[w*(cy)+(cx)]);
			green_chan[w*(cy)+(cx)] = PIXG(src[w*(cy)+(cx)]);
			blue_chan[w*(cy)+(cx)] = PIXB(src[w*(cy)+(cx)]);
		}
	}
	
	memcpy(data, red_chan, w*h);
	memcpy(data+(w*h), green_chan, w*h);
	memcpy(data+((w*h)*2), blue_chan, w*h);
	free(red_chan);
	free(green_chan);
	free(blue_chan);
	
	result[0] = 'P';
	result[1] = 'T';
	result[2] = 'i';
	result[3] = 1;
	result[4] = w;
	result[5] = w>>8;
	result[6] = h;
	result[7] = h>>8;
	
	i -= 8;
	
	if(BZ2_bzBuffToBuffCompress((char *)(result+8), (unsigned *)&i, (char *)data, datalen, 9, 0, 0) != BZ_OK){
		free(data);
		free(result);
		return NULL;
	}
	
	*result_size = i+8;
	free(data);
	return result;
}

pixel *ptif_unpack(void *datain, int size, int *w, int *h){
	int width, height, i, cx, cy, resCode;
	unsigned char *red_chan;
	unsigned char *green_chan;
	unsigned char *blue_chan;
	unsigned char *data = datain;
	unsigned char *undata;
	pixel *result;
	if(size<16){
		printf("Image empty\n");
		return NULL;
	}
	if(!(data[0]=='P' && data[1]=='T' && data[2]=='i')){
		printf("Image header invalid\n");
		return NULL;
	}
	width = data[4]|(data[5]<<8);
	height = data[6]|(data[7]<<8);
	
	i = (width*height)*3;
	undata = calloc(1, (width*height)*3);
	red_chan = calloc(1, width*height); 
	green_chan = calloc(1, width*height); 
	blue_chan = calloc(1, width*height); 
	result = calloc(width*height, PIXELSIZE);
	
	resCode = BZ2_bzBuffToBuffDecompress((char *)undata, (unsigned *)&i, (char *)(data+8), size-8, 0, 0);
	if (resCode){
		printf("Decompression failure, %d\n", resCode);
		free(red_chan);
		free(green_chan);
		free(blue_chan);
		free(undata);
		free(result);
		return NULL;
	}
	if(i != (width*height)*3){
		printf("Result buffer size mismatch, %d != %d\n", i, (width*height)*3);
		free(red_chan);
		free(green_chan);
		free(blue_chan);
		free(undata);
		free(result);
		return NULL;
	}
	memcpy(red_chan, undata, width*height);
	memcpy(green_chan, undata+(width*height), width*height);
	memcpy(blue_chan, undata+((width*height)*2), width*height);
	
	for(cx = 0; cx<width; cx++){
		for(cy = 0; cy<height; cy++){
			result[width*(cy)+(cx)] = PIXRGB(red_chan[width*(cy)+(cx)], green_chan[width*(cy)+(cx)], blue_chan[width*(cy)+(cx)]);
		}
	}
	
	*w = width;
	*h = height;
	free(red_chan);
	free(green_chan);
	free(blue_chan);
	free(undata);
	return result;
}

pixel *resample_img_nn(pixel * src, int sw, int sh, int rw, int rh)
{
	int y, x;
	pixel *q = NULL;
	q = malloc(rw*rh*PIXELSIZE);
	for (y=0; y<rh; y++)
		for (x=0; x<rw; x++){
			q[rw*y+x] = src[sw*(y*sh/rh)+(x*sw/rw)];
		}
	return q;
}

pixel *resample_img(pixel *src, int sw, int sh, int rw, int rh)
{
	int y, x, fxceil, fyceil;
	//int i,j,x,y,w,h,r,g,b,c;
	pixel *q = NULL;
	//TODO: Actual resampling, this is just cheap nearest pixel crap
	if(rw == sw && rh == sh){
		//Don't resample
		q = malloc(rw*rh*PIXELSIZE);
		memcpy(q, src, rw*rh*PIXELSIZE);
	} else if(rw > sw && rh > sh){
		float fx, fy, fyc, fxc;
		double intp;
		pixel tr, tl, br, bl;
		q = malloc(rw*rh*PIXELSIZE);
		//Bilinear interpolation for upscaling
		for (y=0; y<rh; y++)
			for (x=0; x<rw; x++)
			{
				fx = ((float)x)*((float)sw)/((float)rw);
				fy = ((float)y)*((float)sh)/((float)rh);
				fxc = modf(fx, &intp);
				fyc = modf(fy, &intp);
				fxceil = (int)ceil(fx);
				fyceil = (int)ceil(fy);
				if (fxceil>=sw) fxceil = sw-1;
				if (fyceil>=sh) fyceil = sh-1;
				tr = src[sw*(int)floor(fy)+fxceil];
				tl = src[sw*(int)floor(fy)+(int)floor(fx)];
				br = src[sw*fyceil+fxceil];
				bl = src[sw*fyceil+(int)floor(fx)];
				q[rw*y+x] = PIXRGB(
					(int)(((((float)PIXR(tl))*(1.0f-fxc))+(((float)PIXR(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXR(bl))*(1.0f-fxc))+(((float)PIXR(br))*(fxc)))*(fyc)),
					(int)(((((float)PIXG(tl))*(1.0f-fxc))+(((float)PIXG(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXG(bl))*(1.0f-fxc))+(((float)PIXG(br))*(fxc)))*(fyc)),
					(int)(((((float)PIXB(tl))*(1.0f-fxc))+(((float)PIXB(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXB(bl))*(1.0f-fxc))+(((float)PIXB(br))*(fxc)))*(fyc))
					);				
			}
	} else {
		//Stairstepping
		float fx, fy, fyc, fxc;
		double intp;
		pixel tr, tl, br, bl;
		int rrw = rw, rrh = rh;
		pixel * oq;
		oq = malloc(sw*sh*PIXELSIZE);
		memcpy(oq, src, sw*sh*PIXELSIZE);
		rw = sw;
		rh = sh;
		while(rrw != rw && rrh != rh){
			rw *= 0.7;
			rh *= 0.7;
			if(rw <= rrw || rh <= rrh){
				rw = rrw;
				rh = rrh;
			}
			q = malloc(rw*rh*PIXELSIZE);
			//Bilinear interpolation for upscaling
			for (y=0; y<rh; y++)
				for (x=0; x<rw; x++)
				{
					fx = ((float)x)*((float)sw)/((float)rw);
					fy = ((float)y)*((float)sh)/((float)rh);
					fxc = modf(fx, &intp);
					fyc = modf(fy, &intp);
					fxceil = (int)ceil(fx);
					fyceil = (int)ceil(fy);
					if (fxceil>=sw) fxceil = sw-1;
					if (fyceil>=sh) fyceil = sh-1;
					tr = oq[sw*(int)floor(fy)+fxceil];
					tl = oq[sw*(int)floor(fy)+(int)floor(fx)];
					br = oq[sw*fyceil+fxceil];
					bl = oq[sw*fyceil+(int)floor(fx)];
					q[rw*y+x] = PIXRGB(
						(int)(((((float)PIXR(tl))*(1.0f-fxc))+(((float)PIXR(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXR(bl))*(1.0f-fxc))+(((float)PIXR(br))*(fxc)))*(fyc)),
						(int)(((((float)PIXG(tl))*(1.0f-fxc))+(((float)PIXG(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXG(bl))*(1.0f-fxc))+(((float)PIXG(br))*(fxc)))*(fyc)),
						(int)(((((float)PIXB(tl))*(1.0f-fxc))+(((float)PIXB(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXB(bl))*(1.0f-fxc))+(((float)PIXB(br))*(fxc)))*(fyc))
						);				
				}
			free(oq);
			oq = q;
			sw = rw;
			sh = rh;
		}
	}
	return q;
}

pixel *rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f)
{
	int i,j,x,y,w,h,r,g,b,c;
	pixel p, *q;
	w = (sw+f-1)/f;
	h = (sh+f-1)/f;
	q = malloc(w*h*PIXELSIZE);
	for (y=0; y<h; y++)
		for (x=0; x<w; x++)
		{
			r = g = b = c = 0;
			for (j=0; j<f; j++)
				for (i=0; i<f; i++)
					if (x*f+i<sw && y*f+j<sh)
					{
						p = src[(y*f+j)*sw + (x*f+i)];
						if (p)
						{
							r += PIXR(p);
							g += PIXG(p);
							b += PIXB(p);
							c ++;
						}
					}
			if (c>1)
			{
				r = (r+c/2)/c;
				g = (g+c/2)/c;
				b = (b+c/2)/c;
			}
			q[y*w+x] = PIXRGB(r, g, b);
		}
	*qw = w;
	*qh = h;
	return q;
}

#ifdef OGLR
void clearScreen(float alpha)
{
	if(alpha > 0.999f)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    else
    {
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(XRES, 0);
		glVertex2f(XRES, YRES);
		glVertex2f(0, YRES);
		glEnd();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlendEquation(GL_FUNC_ADD);
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void clearScreenNP(float alpha)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void ogl_blit(int x, int y, int w, int h, pixel *src, int pitch, int scale)
{

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDrawPixels(w,h,GL_BGRA,GL_UNSIGNED_BYTE,src); //Why does this still think it's ABGR?
    glEnable( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, vidBuf);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES+BARSIZE, YRES+MENUSIZE, GL_BGRA, GL_UNSIGNED_BYTE, src);
	glBegin(GL_QUADS);
    glTexCoord2d(1, 0);
    glVertex3f((XRES+BARSIZE)*sdl_scale, (YRES+MENUSIZE)*sdl_scale, 1.0);
    glTexCoord2d(0, 0);
    glVertex3f(0, (YRES+MENUSIZE)*sdl_scale, 1.0);
    glTexCoord2d(0, 1);
    glVertex3f(0, 0, 1.0);
    glTexCoord2d(1, 1);
    glVertex3f((XRES+BARSIZE)*sdl_scale, 0, 1.0);
    glEnd();

    glDisable( GL_TEXTURE_2D );
	glBlendFunc(GL_ONE, GL_ONE);
    glFlush();
    SDL_GL_SwapBuffers ();
}
#endif

void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch)
{
	pixel *dst;
	int j;
	if (SDL_MUSTLOCK(sdl_scrn))
		if (SDL_LockSurface(sdl_scrn)<0)
			return;
	dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
	if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
	{
		//pixel format conversion
		int i;
		pixel px;
		SDL_PixelFormat *fmt = sdl_scrn->format;
		for (j=0; j<h; j++)
		{
			for (i=0; i<w; i++)
			{
				px = src[i];
				dst[i] = ((PIXR(px)>>fmt->Rloss)<<fmt->Rshift)|
						((PIXG(px)>>fmt->Gloss)<<fmt->Gshift)|
						((PIXB(px)>>fmt->Bloss)<<fmt->Bshift);
			}
			dst+=sdl_scrn->pitch/PIXELSIZE;
			src+=pitch;
		}
	}
	else
	{
		for (j=0; j<h; j++)
		{
			memcpy(dst, src, w*PIXELSIZE);
			dst+=sdl_scrn->pitch/PIXELSIZE;
			src+=pitch;
		}
	}
	if (SDL_MUSTLOCK(sdl_scrn))
		SDL_UnlockSurface(sdl_scrn);
	SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void sdl_blit_2(int x, int y, int w, int h, pixel *src, int pitch)
{
	pixel *dst;
	int j;
	int i,k;
	if (SDL_MUSTLOCK(sdl_scrn))
		if (SDL_LockSurface(sdl_scrn)<0)
			return;
	dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
	if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
	{
		//pixel format conversion
		pixel px;
		SDL_PixelFormat *fmt = sdl_scrn->format;
		for (j=0; j<h; j++)
		{
			for (k=0; k<sdl_scale; k++)
			{
				for (i=0; i<w; i++)
				{
					px = src[i];
					px = ((PIXR(px)>>fmt->Rloss)<<fmt->Rshift)|
						((PIXG(px)>>fmt->Gloss)<<fmt->Gshift)|
						((PIXB(px)>>fmt->Bloss)<<fmt->Bshift);
					dst[i*2]=px;
					dst[i*2+1]=px;
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
			}
			src+=pitch;
		}
	}
	else
	{
		for (j=0; j<h; j++)
		{
			for (k=0; k<sdl_scale; k++)
			{
				for (i=0; i<w; i++)
				{
					dst[i*2]=src[i];
					dst[i*2+1]=src[i];
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
			}
			src+=pitch;
		}
	}
	if (SDL_MUSTLOCK(sdl_scrn))
		SDL_UnlockSurface(sdl_scrn);
	SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void sdl_blit(int x, int y, int w, int h, pixel *src, int pitch)
{
#if defined(OGLR)
        ogl_blit(x, y, w, h, src, pitch, sdl_scale);
#else
	if (sdl_scale == 2)
		sdl_blit_2(x, y, w, h, src, pitch);
	else
		sdl_blit_1(x, y, w, h, src, pitch);
#endif
}

//an easy way to draw a blob
void drawblob(pixel *vid, int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
	blendpixel(vid, x+1, y, cr, cg, cb, 112);
	blendpixel(vid, x-1, y, cr, cg, cb, 112);
	blendpixel(vid, x, y+1, cr, cg, cb, 112);
	blendpixel(vid, x, y-1, cr, cg, cb, 112);

	blendpixel(vid, x+1, y-1, cr, cg, cb, 64);
	blendpixel(vid, x-1, y-1, cr, cg, cb, 64);
	blendpixel(vid, x+1, y+1, cr, cg, cb, 64);
	blendpixel(vid, x-1, y+1, cr, cg, cb, 64);
}

//draws walls and elements for menu
int draw_tool_xy(pixel *vid_buf, int x, int y, int b, unsigned pc)
{
	int i, j, c;
	pixel gc;
	if (x > XRES-26 || x < 0)
		return 26;
	if ((b&0xFF) == PT_LIFE)
	{
		for (j=1; j<15; j++)
		{
			for (i=1; i<27; i++)
			{
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
			}
		}
		c = PIXB(pc) + 3*PIXG(pc) + 2*PIXR(pc);
		if (c<544)
		{
			c = 255;
		}
		else
		{
			c = 0;
		}
		drawtext(vid_buf, x+14-textwidth((char *)gmenu[(b>>8)&0xFF].name)/2, y+4, (char *)gmenu[(b>>8)&0xFF].name, c, c, c, 255);
	}
	else if (b>=UI_WALLSTART)
	{
		int ds = 0;
		if (b-UI_WALLSTART>=0 && b-UI_WALLSTART<UI_WALLCOUNT)
		{
			ds = wtypes[b-UI_WALLSTART].drawstyle;
			gc = wtypes[b-UI_WALLSTART].eglow;
		}
		//x = (2+32*((b-22)/1));
		//y = YRES+2+40;
		if (ds==1)
		{
			for (j=1; j<15; j+=2)
				for (i=1+(1&(j>>1)); i<27; i+=2)
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
		}
		else if (ds==2)
		{
			for (j=1; j<15; j+=2)
				for (i=1; i<27; i+=2)
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
		}
		else if (ds==3)
		{
			for (j=1; j<15; j++)
				for (i=1; i<27; i++)
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
		}
		else if (ds==4)
		{
			for (j=1; j<15; j++)
				for (i=1; i<27; i++)
					if(i%CELL == j%CELL)
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
					else if  (i%CELL == (j%CELL)+1 || (i%CELL == 0 && j%CELL == CELL-1))
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = gc;
					else 
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = PIXPACK(0x202020);
		}
		else
		switch (b)
		{
		case WL_WALLELEC+100:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					if (!(i%2) && !(j%2))
					{
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
					}
					else
					{
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = PIXPACK(0x808080);
					}
				}
			}
			break;
		case WL_EWALL+100:
			for (j=1; j<15; j++)
			{
				for (i=1; i<6+j; i++)
				{
					if (!(i&j&1))
					{
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
					}
				}
				for (; i<27; i++)
				{
					if (i&j&1)
					{
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
					}
				}
			}
			break;
		case WL_STREAM+100:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
					drawtext(vid_buf, x+4, y+3, "\x8D", 255, 255, 255, 255);
				}
			}
			for (i=9; i<27; i++)
			{
				drawpixel(vid_buf, x+i, y+8+(int)(3.9f*cos(i*0.3f)), 255, 255, 255, 255);
			}
			break;
		case WL_SIGN+100:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
				}
			}
			drawtext(vid_buf, x+9, y+3, "\xA1", 32, 64, 128, 255);
			drawtext(vid_buf, x+9, y+3, "\xA0", 255, 255, 255, 255);
			break;
		case WL_ERASE+100:
			for (j=1; j<15; j+=2)
			{
				for (i=1+(1&(j>>1)); i<13; i+=2)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			for (j=1; j<15; j++)
			{
				for (i=14; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			break;
		case SPC_AIR:
		case SPC_HEAT:
		case SPC_COOL:
		case SPC_VACUUM:
		case SPC_WIND:
		case SPC_PGRV:
		case SPC_NGRV:
		case SPC_PROP:
			for (j=1; j<15; j++)
				for (i=1; i<27; i++)
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
			c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
			if (c<544)
			{
				c = 255;
			}
			else
			{
				c = 0;
			}
			if (b==SPC_AIR)
				drawtext(vid_buf, x+14-textwidth("AIR")/2, y+4, "AIR", c, c, c, 255);
			else if (b==SPC_HEAT)
				drawtext(vid_buf, x+14-textwidth("HEAT")/2, y+4, "HEAT", c, c, c, 255);
			else if (b==SPC_COOL)
				drawtext(vid_buf, x+14-textwidth("COOL")/2, y+4, "COOL", c, c, c, 255);
			else if (b==SPC_VACUUM)
				drawtext(vid_buf, x+14-textwidth("VAC")/2, y+4, "VAC", c, c, c, 255);
			else if (b==SPC_WIND)
				drawtext(vid_buf, x+14-textwidth("WIND")/2, y+4, "WIND", c, c, c, 255);
			else if (b==SPC_PGRV)
				drawtext(vid_buf, x+14-textwidth("PGRV")/2, y+4, "PGRV", c, c, c, 255);
			else if (b==SPC_NGRV)
				drawtext(vid_buf, x+14-textwidth("NGRV")/2, y+4, "NGRV", c, c, c, 255);
			else if (b==SPC_PROP)
				drawtext(vid_buf, x+14-textwidth("PROP")/2, y+4, "PROP", c, c, c, 255);
			break;
		default:
			for (j=1; j<15; j++)
				for (i=1; i<27; i++)
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
		}
		if (b==WL_ERASE+100)
		{
			for (j=4; j<12; j++)
			{
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
				vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
				vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
			}
		}
	}
	else
	{
		//x = 2+32*(b/2);
		//y = YRES+2+20*(b%2);
		for (j=1; j<15; j++)
		{
			for (i=1; i<27; i++)
			{
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
			}
		}
		if (b==0)
		{
			for (j=4; j<12; j++)
			{
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
				vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
				vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
			}
		}
		c = PIXB(ptypes[b].pcolors) + 3*PIXG(ptypes[b].pcolors) + 2*PIXR(ptypes[b].pcolors);
		if (c<544)
		{
			c = 255;
		}
		else
		{
			c = 0;
		}
		drawtext(vid_buf, x+14-textwidth((char *)ptypes[b].name)/2, y+4, (char *)ptypes[b].name, c, c, c, 255);
	}
	return 26;
}

void draw_menu(pixel *vid_buf, int i, int hover)
{
	if (i==SEC&&SEC!=0)
		drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 0, 255, 255, 255);
	else
		drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
	if (hover==i)
	{
		fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
		drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 0, 0, 0, 255);
	}
	else
	{
		drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 255, 255, 255, 255);
	}
}

void draw_color_menu(pixel *vid_buf, int i, int hover)
{
	drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(DECO_SECTIONS*16), 14, 14, 255, 255, 255, 255);
	if (hover==i)
	{
		fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(DECO_SECTIONS*16), 14, 14, 255, 255, 255, 255);
		drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(DECO_SECTIONS*16), colorsections[i].icon, 0, 0, 0, 255);
	}
	else
	{
		drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(DECO_SECTIONS*16), colorsections[i].icon, 255, 255, 255, 255);
	}
}

//draws a pixel, identical to blendpixel(), except blendpixel has OpenGL support
#if defined(WIN32) && !defined(__GNUC__)
_inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#else
inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#endif
{
#ifdef PIXALPHA
	pixel t;
	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return;
	if (a!=255)
	{
		t = vid[y*(XRES+BARSIZE)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
		a = a > PIXA(t) ? a : PIXA(t);
	}
	vid[y*(XRES+BARSIZE)+x] = PIXRGBA(r,g,b,a);
#else
	pixel t;
	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return;
	if (a!=255)
	{
		t = vid[y*(XRES+BARSIZE)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
	}
	vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
#endif
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a)
#else
inline int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a)
#endif
{
	int i, j, w, bn = 0, ba = 0;
	char *rp = font_data + font_ptrs[c];
	w = *(rp++);
	for (j=0; j<FONT_H; j++)
		for (i=0; i<w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			drawpixel(vid, x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

int addchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a)
{
	int i, j, w, bn = 0, ba = 0;
	char *rp = font_data + font_ptrs[c];
	w = *(rp++);
	for (j=0; j<FONT_H; j++)
		for (i=0; i<w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			{
			addpixel(vid, x+i, y+j, r, g, b, ((ba&3)*a)/3);
			}
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

int drawtext(pixel *vid, int x, int y, const char *s, int r, int g, int b, int a)
{
	int sx = x;
	for (; *s; s++)
	{
		if (*s == '\n')
		{
			x = sx;
			y += FONT_H+2;
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
			x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
	}
	return x;
}

//Draw text with an outline
int drawtext_outline(pixel *vid, int x, int y, const char *s, int r, int g, int b, int a, int or, int og, int ob, int oa)
{
	drawtext(vid, x-1, y-1, s, or, og, ob, oa);
	drawtext(vid, x+1, y+1, s, or, og, ob, oa);
	
	drawtext(vid, x-1, y+1, s, or, og, ob, oa);
	drawtext(vid, x+1, y-1, s, or, og, ob, oa);
	
	return drawtext(vid, x, y, s, r, g, b, a);
}
int drawtextwrap(pixel *vid, int x, int y, int w, const char *s, int r, int g, int b, int a)
{
	int sx = x;
	int rh = 12;
	int rw = 0;
	int cw = x;
	int wordlen;
	int charspace;
	while (*s)
	{
		wordlen = strcspn(s," .,!?\n");
		charspace = textwidthx((char *)s, w-(x-cw));
		if (charspace<wordlen && wordlen && w-(x-cw)<w/3)
		{
			x = sx;
			rw = 0;
			y+=FONT_H+2;
			rh+=FONT_H+2;
		}
		for (; *s && --wordlen>=-1; s++)
		{
			if (*s == '\n')
			{
				x = sx;
				rw = 0;
				y += FONT_H+2;
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

				if (x-cw>=w)
				{
					x = sx;
					rw = 0;
					y+=FONT_H+2;
					rh+=FONT_H+2;
					if (*s==' ')
						continue;
				}
				x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
			}
		}
	}

	return rh;
}

//draws a rectange, (x,y) are the top left coords.
void drawrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i;
	for (i=0; i<=w; i++)
	{
		drawpixel(vid, x+i, y, r, g, b, a);
		drawpixel(vid, x+i, y+h, r, g, b, a);
	}
	for (i=1; i<h; i++)
	{
		drawpixel(vid, x, y+i, r, g, b, a);
		drawpixel(vid, x+w, y+i, r, g, b, a);
	}
}

//draws a rectangle and fills it in as well.
void fillrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i,j;
	for (j=1; j<h; j++)
		for (i=1; i<w; i++)
			drawpixel(vid, x+i, y+j, r, g, b, a);
}

void clearrect(pixel *vid, int x, int y, int w, int h)
{
#ifdef OGLR
	fillrect(vid, x, y, w, h, 0, 0, 0, 255);
#else
	int i;
	for (i=1; i<h; i++)
		memset(vid+(x+1+(XRES+BARSIZE)*(y+i)), 0, PIXELSIZE*(w-1));
#endif
}
//draws a line of dots, where h is the height. (why is this even here)
void drawdots(pixel *vid, int x, int y, int h, int r, int g, int b, int a)
{
	int i;
	for (i=0; i<=h; i+=2)
		drawpixel(vid, x, y+i, r, g, b, a);
}

int textwidth(char *s)
{
	int x = 0;
	for (; *s; s++)
		x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
	return x-1;
}

int drawtextmax(pixel *vid, int x, int y, int w, char *s, int r, int g, int b, int a)
{
	int i;
	w += x-5;
	for (; *s; s++)
	{
		if (x+font_data[font_ptrs[(int)(*(unsigned char *)s)]]>=w && x+textwidth(s)>=w+5)
			break;
		x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
	}
	if (*s)
		for (i=0; i<3; i++)
			x = drawchar(vid, x, y, '.', r, g, b, a);
	return x;
}

int textnwidth(char *s, int n)
{
	int x = 0;
	for (; *s; s++)
	{
		if (!n)
			break;
		x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
		n--;
	}
	return x-1;
}
void textnpos(char *s, int n, int w, int *cx, int *cy)
{
	int x = 0;
	int y = 0;
	int wordlen, charspace;
	while (*s&&n)
	{
		wordlen = strcspn(s," .,!?\n");
		charspace = textwidthx(s, w-x);
		if (charspace<wordlen && wordlen && w-x<w/3)
		{
			x = 0;
			y += FONT_H+2;
		}
		for (; *s && --wordlen>=-1; s++)
		{
			if (!n) {
				break;
			}
			x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
			if (x>=w)
			{
				x = 0;
				y += FONT_H+2;
				if (*(s+1)==' ')
					x -= font_data[font_ptrs[(int)(' ')]];
			}
			n--;
		}
	}
	*cx = x-1;
	*cy = y;
}

int textwidthx(char *s, int w)
{
	int x=0,n=0,cw;
	for (; *s; s++)
	{
		cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
		if (x+(cw/2) >= w)
			break;
		x += cw;
		n++;
	}
	return n;
}
int textposxy(char *s, int width, int w, int h)
{
	int x=0,y=0,n=0,cw, wordlen, charspace;
	while (*s)
	{
		wordlen = strcspn(s," .,!?\n");
		charspace = textwidthx(s, width-x);
		if (charspace<wordlen && wordlen && width-x<width/3)
		{
			x = 0;
			y += FONT_H+2;
		}
		for (; *s && --wordlen>=-1; s++)
		{
			cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
			if ((x+(cw/2) >= w && y+6 >= h)||(y+6 >= h+FONT_H+2))
				return n++;
			x += cw;
			if (x>=width) {
				x = 0;
				y += FONT_H+2;
				if (*(s+1)==' ')
					x -= font_data[font_ptrs[(int)(' ')]];
			}
			n++;
		}
	}
	return n;
}
int textwrapheight(char *s, int width)
{
	int x=0, height=FONT_H+2, cw;
	int wordlen;
	int charspace;
	while (*s)
	{
		wordlen = strcspn(s," .,!?\n");
		charspace = textwidthx(s, width-x);
		if (charspace<wordlen && wordlen && width-x<width/3)
		{
			x = 0;
			height += FONT_H+2;
		}
		for (; *s && --wordlen>=-1; s++)
		{
			if (*s == '\n')
			{
				x = 0;
				height += FONT_H+2;
			}
			else if (*s == '\b')
			{
				s++;
			}
			else
			{
				cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
				if (x+cw>=width)
				{
					x = 0;
					height += FONT_H+2;
					if (*s==' ')
						continue;
				}
				x += cw;
			}
		}
	}
	return height;
}

//the most used function for drawing a pixel, because it has OpenGL support, which is not fully implemented.
#if defined(WIN32) && !defined(__GNUC__)
_inline void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#else
inline void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#endif
{
#ifdef PIXALPHA
	pixel t;
	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return;
	if (a!=255)
	{
		t = vid[y*(XRES+BARSIZE)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
		a = a > PIXA(t) ? a : PIXA(t);
	}
	vid[y*(XRES+BARSIZE)+x] = PIXRGBA(r,g,b,a);
#else
	pixel t;
	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return;
	if (a!=255)
	{
		t = vid[y*(XRES+BARSIZE)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
	}
	vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
#endif
}

void draw_icon(pixel *vid_buf, int x, int y, char ch, int flag)
{
	char t[2];
	t[0] = ch;
	t[1] = 0;
	if (flag)
	{
		fillrect(vid_buf, x-1, y-1, 17, 17, 255, 255, 255, 255);
		drawtext(vid_buf, x+3, y+2, t, 0, 0, 0, 255);
	}
	else
	{
		drawrect(vid_buf, x, y, 15, 15, 255, 255, 255, 255);
		drawtext(vid_buf, x+3, y+2, t, 255, 255, 255, 255);
	}
}
void draw_air(pixel *vid)
{
#ifndef OGLR
	int x, y, i, j;
	pixel c;
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			if (display_mode & DISPLAY_AIRP)
			{
				if (pv[y][x] > 0.0f)
					c  = PIXRGB(clamp_flt(pv[y][x], 0.0f, 8.0f), 0, 0);//positive pressure is red!
				else
					c  = PIXRGB(0, 0, clamp_flt(-pv[y][x], 0.0f, 8.0f));//negative pressure is blue!
			}
			else if (display_mode & DISPLAY_AIRV)
			{
				c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),//vx adds red
					clamp_flt(pv[y][x], 0.0f, 8.0f),//pressure adds green
					clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));//vy adds blue
			}
			else if ((display_mode & DISPLAY_AIRH))
			{
				if (aheat_enable)
				{
					float ttemp = hv[y][x]+(-MIN_TEMP);
					int caddress = restrict_flt((int)( restrict_flt(ttemp, 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
					c = PIXRGB((int)((unsigned char)color_data[caddress]*0.7f), (int)((unsigned char)color_data[caddress+1]*0.7f), (int)((unsigned char)color_data[caddress+2]*0.7f));
					//c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),//vx adds red
					//	clamp_flt(hv[y][x], 0.0f, 1600.0f),//heat adds green
					//	clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));//vy adds blue
				}
				else
					c = PIXRGB(0,0,0);
			}
			else if (display_mode & DISPLAY_AIRC)
			{
				int r;
				int g;
				int b;
				// velocity adds grey
				r = clamp_flt(fabsf(vx[y][x]), 0.0f, 24.0f) + clamp_flt(fabsf(vy[y][x]), 0.0f, 20.0f);
				g = clamp_flt(fabsf(vx[y][x]), 0.0f, 20.0f) + clamp_flt(fabsf(vy[y][x]), 0.0f, 24.0f);
				b = clamp_flt(fabsf(vx[y][x]), 0.0f, 24.0f) + clamp_flt(fabsf(vy[y][x]), 0.0f, 20.0f);
				if (pv[y][x] > 0.0f)
				{
					r += clamp_flt(pv[y][x], 0.0f, 16.0f);//pressure adds red!
					if (r>255)
						r=255;
					if (g>255)
						g=255;
					if (b>255)
						b=255;
					c  = PIXRGB(r, g, b);
				}
				else
				{
					b += clamp_flt(-pv[y][x], 0.0f, 16.0f);//pressure adds blue!
					if (r>255)
						r=255;
					if (g>255)
						g=255;
					if (b>255)
						b=255;
					c  = PIXRGB(r, g, b);
				}
			}
			for (j=0; j<CELL; j++)//draws the colors
				for (i=0; i<CELL; i++)
					vid[(x*CELL+i) + (y*CELL+j)*(XRES+BARSIZE)] = c;
		}
#else
	GLuint airProg;
	if(display_mode & DISPLAY_AIRC)
	{
		airProg = airProg_Cracker;
	}
	else if(display_mode & DISPLAY_AIRV)
	{
		airProg = airProg_Velocity;
	}
	else if(display_mode & DISPLAY_AIRP)
	{
		airProg = airProg_Pressure;
	}
	else
	{
		return;
	}

    glEnable( GL_TEXTURE_2D );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
    
	glUseProgram(airProg);
	
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, airVX);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_RED, GL_FLOAT, vx);
    glUniform1i(glGetUniformLocation(airProg, "airX"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, airVY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_GREEN, GL_FLOAT, vy);
    glUniform1i(glGetUniformLocation(airProg, "airY"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, airPV);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_BLUE, GL_FLOAT, pv);
    glUniform1i(glGetUniformLocation(airProg, "airP"), 2);
    glActiveTexture(GL_TEXTURE0);
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
    glTexCoord2d(1, 1);
    glVertex3f(XRES*sdl_scale, YRES*sdl_scale, 1.0);
    glTexCoord2d(0, 1);
    glVertex3f(0, YRES*sdl_scale, 1.0);
    glTexCoord2d(0, 0);
    glVertex3f(0, 0, 1.0);
    glTexCoord2d(1, 0);
    glVertex3f(XRES*sdl_scale, 0, 1.0);
    glEnd();
    
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable( GL_TEXTURE_2D );
#endif
}

void draw_grav_zones(pixel * vid)
{
	int x, y, i, j;
	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			if(gravmask[y*(XRES/CELL)+x])
			{
				for (j=0; j<CELL; j++)//draws the colors
					for (i=0; i<CELL; i++)
						if(i == j)
							drawpixel(vid, x*CELL+i, y*CELL+j, 255, 200, 0, 120);
						else 
							drawpixel(vid, x*CELL+i, y*CELL+j, 32, 32, 32, 120);
			}
		}
	}
}

void draw_grav(pixel *vid)
{
	int x, y, i, ca;
	float nx, ny, dist;

	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			ca = y*(XRES/CELL)+x;
			if(fabsf(gravx[ca]) <= 0.001f && fabsf(gravy[ca]) <= 0.001f)
				continue;
			nx = x*CELL;
			ny = y*CELL;
			dist = fabsf(gravy[ca])+fabsf(gravx[ca]);
			for(i = 0; i < 4; i++)
			{
				nx -= gravx[ca]*0.5f;
				ny -= gravy[ca]*0.5f;
				addpixel(vid, (int)(nx+0.5f), (int)(ny+0.5f), 255, 255, 255, (int)(dist*20.0f));
			}
		}
	}
}

void draw_line(pixel *vid, int x1, int y1, int x2, int y2, int r, int g, int b, int a)  //Draws a line
{
	int dx, dy, i, sx, sy, check, e, x, y;

	dx = abs(x1-x2);
	dy = abs(y1-y2);
	sx = isign(x2-x1);
	sy = isign(y2-y1);
	x = x1;
	y = y1;
	check = 0;

	if (dy>dx)
	{
		dx = dx+dy;
		dy = dx-dy;
		dx = dx-dy;
		check = 1;
	}

	e = (dy<<2)-dx;
	for (i=0; i<=dx; i++)
	{
		if (x>=0 && y>=0 && x<a && y<YRES+MENUSIZE)
			vid[x+y*a] =PIXRGB(r, g, b);
		if (e>=0)
		{
			if (check==1)
				x = x+sx;
			else
				y = y+sy;
			e = e-(dx<<2);
		}
		if (check==1)
			y = y+sy;
		else
			x = x+sx;
		e = e+(dy<<2);
	}
}

//adds color to a pixel, does not overwrite.
void addpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
{
	pixel t;
	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return;
	t = vid[y*(XRES+BARSIZE)+x];
	r = (a*r + 255*PIXR(t)) >> 8;
	g = (a*g + 255*PIXG(t)) >> 8;
	b = (a*b + 255*PIXB(t)) >> 8;
	if (r>255)
		r = 255;
	if (g>255)
		g = 255;
	if (b>255)
		b = 255;
	vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
}

//draws one of two colors, so that it is always clearly visible
void xor_pixel(int x, int y, pixel *vid)
{
	int c;
	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return;
	c = vid[y*(XRES+BARSIZE)+x];
	c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
	if (c<512)
		vid[y*(XRES+BARSIZE)+x] = PIXPACK(0xC0C0C0);
	else
		vid[y*(XRES+BARSIZE)+x] = PIXPACK(0x404040);
}

//same as xor_pixel, but draws a line of it
void xor_line(int x1, int y1, int x2, int y2, pixel *vid)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (cp)
			xor_pixel(y, x, vid);
		else
			xor_pixel(x, y, vid);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

//same as blend_pixel, but draws a line of it
void blend_line(pixel *vid, int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
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

//same as xor_pixel, but draws a rectangle
void xor_rect(pixel *vid, int x, int y, int w, int h)
{
	int i;
	for (i=0; i<w; i+=2)
	{
		xor_pixel(x+i, y, vid);
	}
	if (h != 1)
	{
		if (h%2 == 1) i = 2;
		else i = 1;
		for (; i<w; i+=2)
		{
			xor_pixel(x+i, y+h-1, vid);
		}
	}

	for (i=2; i<h; i+=2)
	{
		xor_pixel(x, y+i, vid);
	}
	if (w != 1)
	{
		if (w%2 == 1) i = 2;
		else i = 1;
		for (; i<h-1; i+=2)
		{
			xor_pixel(x+w-1, y+i, vid);
		}
	}
}

void draw_other(pixel *vid) // EMP effect
{
	int i, j;
	if (emp_decor>0 && !sys_pause) emp_decor-=emp_decor/25+2;
	if (emp_decor>40) emp_decor=40;
	if (emp_decor<0) emp_decor = 0;
	if (!(render_mode & EFFECT)) // not in nothing mode
		return;
	if (emp_decor>0)
	{
#ifdef OGLR
		float femp_decor = ((float)emp_decor)/255.0f;
		/*int r=emp_decor*2.5, g=100+emp_decor*1.5, b=255;
		int a=(1.0*emp_decor/110)*255;
		if (r>255) r=255;
		if (g>255) g=255;
		if (b>255) g=255;
		if (a>255) a=255;*/
		glBegin(GL_QUADS);
		glColor4f(femp_decor*2.5f, 0.4f+femp_decor*1.5f, 1.0f+femp_decor*1.5f, femp_decor/0.44f);
		glVertex2f(0, MENUSIZE);
		glVertex2f(XRES, MENUSIZE);
		glVertex2f(XRES, YRES+MENUSIZE);
		glVertex2f(0, YRES+MENUSIZE);
		glEnd();
#else
		int r=emp_decor*2.5, g=100+emp_decor*1.5, b=255;
		int a=(1.0*emp_decor/110)*255;
		if (r>255) r=255;
		if (g>255) g=255;
		if (b>255) g=255;
		if (a>255) a=255;
		for (j=0; j<YRES; j++)
			for (i=0; i<XRES; i++)
			{
				drawpixel(vid, i, j, r, g, b, a);
			}
#endif
	}
}

void prepare_graphicscache()
{
	graphicscache = malloc(sizeof(gcache_item)*PT_NUM);
	memset(graphicscache, 0, sizeof(gcache_item)*PT_NUM);
}
//New function for drawing particles
#ifdef OGLR
GLuint fireV[(YRES*XRES)*2];
GLfloat fireC[(YRES*XRES)*4];
GLuint smokeV[(YRES*XRES)*2];
GLfloat smokeC[(YRES*XRES)*4];
GLuint blobV[(YRES*XRES)*2];
GLfloat blobC[(YRES*XRES)*4];
GLuint blurV[(YRES*XRES)*2];
GLfloat blurC[(YRES*XRES)*4];
GLuint glowV[(YRES*XRES)*2];
GLfloat glowC[(YRES*XRES)*4];
GLuint flatV[(YRES*XRES)*2];
GLfloat flatC[(YRES*XRES)*4];
GLuint addV[(YRES*XRES)*2];
GLfloat addC[(YRES*XRES)*4];
GLfloat lineV[(((YRES*XRES)*2)*6)];
GLfloat lineC[(((YRES*XRES)*2)*6)];
GLfloat blurLineV[(((YRES*XRES)*2))];
GLfloat blurLineC[(((YRES*XRES)*2)*4)];
GLfloat ablurLineV[(((YRES*XRES)*2))];
GLfloat ablurLineC[(((YRES*XRES)*2)*4)];
#endif
void render_parts(pixel *vid)
{
	int deca, decr, decg, decb, cola, colr, colg, colb, firea, firer, fireg, fireb, pixel_mode, q, i, t, nx, ny, x, y, caddress;
	int orbd[4] = {0, 0, 0, 0}, orbl[4] = {0, 0, 0, 0};
	float gradv, flicker, fnx, fny, flx, fly;
#ifdef OGLR
	int cfireV = 0, cfireC = 0, cfire = 0;
	int csmokeV = 0, csmokeC = 0, csmoke = 0;
	int cblobV = 0, cblobC = 0, cblob = 0;
	int cblurV = 0, cblurC = 0, cblur = 0;
	int cglowV = 0, cglowC = 0, cglow = 0;
	int cflatV = 0, cflatC = 0, cflat = 0;
	int caddV = 0, caddC = 0, cadd = 0;
	int clineV = 0, clineC = 0, cline = 0;
	int cblurLineV = 0, cblurLineC = 0, cblurLine = 0;
	int cablurLineV = 0, cablurLineC = 0, cablurLine = 0;
	GLuint origBlendSrc, origBlendDst;
	
	glGetIntegerv(GL_BLEND_SRC, &origBlendSrc);
	glGetIntegerv(GL_BLEND_DST, &origBlendDst);
	//Render to the particle FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
#else
	if (GRID_MODE)//draws the grid
	{
		for (ny=0; ny<YRES; ny++)
			for (nx=0; nx<XRES; nx++)
			{
				if (ny%(4*GRID_MODE)==0)
					blendpixel(vid, nx, ny, 100, 100, 100, 80);
				if (nx%(4*GRID_MODE)==0)
					blendpixel(vid, nx, ny, 100, 100, 100, 80);
			}
	}
#endif
	for(i = 0; i<=parts_lastActiveIndex; i++) {
		if (parts[i].type) {
			t = parts[i].type;

			nx = (int)(parts[i].x+0.5f);
			ny = (int)(parts[i].y+0.5f);
			fnx = parts[i].x;
			fny = parts[i].y;
#ifdef OGLR
			flx = parts[i].lastX;
			fly = parts[i].lastY;
#endif


			if(photons[ny][nx]&0xFF && !(ptypes[t].properties & TYPE_ENERGY) && t!=PT_STKM && t!=PT_STKM2 && t!=PT_FIGH)
				continue;
				
			//Defaults
			pixel_mode = 0 | PMODE_FLAT;
			cola = 255;
			colr = PIXR(ptypes[t].pcolors);
			colg = PIXG(ptypes[t].pcolors);
			colb = PIXB(ptypes[t].pcolors);
			firea = 0;
			
			deca = (parts[i].dcolour>>24)&0xFF;
			decr = (parts[i].dcolour>>16)&0xFF;
			decg = (parts[i].dcolour>>8)&0xFF;
			decb = (parts[i].dcolour)&0xFF;
				
			/*if(display_mode == RENDER_NONE)
			{
				if(decorations_enable)
				{
					colr = (deca*decr + (255-deca)*colr) >> 8;
					colg = (deca*decg + (255-deca)*colg) >> 8;
					colb = (deca*decb + (255-deca)*colb) >> 8;
				}
#ifdef OGLR
		        flatV[cflatV++] = nx;
		        flatV[cflatV++] = ny;
		        flatC[cflatC++] = ((float)colr)/255.0f;
		        flatC[cflatC++] = ((float)colg)/255.0f;
		        flatC[cflatC++] = ((float)colb)/255.0f;
		        flatC[cflatC++] = 1.0f;
		        cflat++;
#else
		        vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(colr,colg,colb);
#endif
			}
			else*/
			{	
				if (graphicscache[t].isready)
				{
					pixel_mode = graphicscache[t].pixel_mode;
					cola = graphicscache[t].cola;
					colr = graphicscache[t].colr;
					colg = graphicscache[t].colg;
					colb = graphicscache[t].colb;
					firea = graphicscache[t].firea;
					firer = graphicscache[t].firer;
					fireg = graphicscache[t].fireg;
					fireb = graphicscache[t].fireb;
				}
				else if(!(colour_mode & COLOUR_BASC))	//Don't get special effects for BASIC colour mode
				{
#ifdef LUACONSOLE
					if (lua_gr_func[t])
					{
						if (luacon_graphics_update(t,i, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb))
						{
							graphicscache[t].isready = 1;
							graphicscache[t].pixel_mode = pixel_mode;
							graphicscache[t].cola = cola;
							graphicscache[t].colr = colr;
							graphicscache[t].colg = colg;
							graphicscache[t].colb = colb;
							graphicscache[t].firea = firea;
							graphicscache[t].firer = firer;
							graphicscache[t].fireg = fireg;
							graphicscache[t].fireb = fireb;
						}
					}
					else if (ptypes[t].graphics_func)
					{
#else
					if (ptypes[t].graphics_func)
					{
#endif
						if ((*(ptypes[t].graphics_func))(&(parts[i]), nx, ny, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb)) //That's a lot of args, a struct might be better
						{
							graphicscache[t].isready = 1;
							graphicscache[t].pixel_mode = pixel_mode;
							graphicscache[t].cola = cola;
							graphicscache[t].colr = colr;
							graphicscache[t].colg = colg;
							graphicscache[t].colb = colb;
							graphicscache[t].firea = firea;
							graphicscache[t].firer = firer;
							graphicscache[t].fireg = fireg;
							graphicscache[t].fireb = fireb;
						}
					}
					else
					{
						if(graphics_DEFAULT(&(parts[i]), nx, ny, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb))
						{
							graphicscache[t].isready = 1;
							graphicscache[t].pixel_mode = pixel_mode;
							graphicscache[t].cola = cola;
							graphicscache[t].colr = colr;
							graphicscache[t].colg = colg;
							graphicscache[t].colb = colb;
							graphicscache[t].firea = firea;
							graphicscache[t].firer = firer;
							graphicscache[t].fireg = fireg;
							graphicscache[t].fireb = fireb;
						}
					}
				}
				if(ptypes[t].properties & PROP_HOT_GLOW && parts[i].temp>(ptransitions[t].thv-800.0f))
				{
					gradv = 3.1415/(2*ptransitions[t].thv-(ptransitions[t].thv-800.0f));
					caddress = (parts[i].temp>ptransitions[t].thv)?ptransitions[t].thv-(ptransitions[t].thv-800.0f):parts[i].temp-(ptransitions[t].thv-800.0f);
					colr += sin(gradv*caddress) * 226;;
					colg += sin(gradv*caddress*4.55 +3.14) * 34;
					colb += sin(gradv*caddress*2.22 +3.14) * 64;
				}
				
				if(pixel_mode & FIRE_ADD && !(render_mode & FIRE_ADD))
					pixel_mode |= PMODE_GLOW;
				if(pixel_mode & FIRE_BLEND && !(render_mode & FIRE_BLEND))
					pixel_mode |= PMODE_BLUR;
				if(pixel_mode & PMODE_BLUR && !(render_mode & PMODE_BLUR))
					pixel_mode |= PMODE_FLAT;
				if(pixel_mode & PMODE_GLOW && !(render_mode & PMODE_GLOW))
					pixel_mode |= PMODE_BLEND;
				if (render_mode & PMODE_BLOB)
					pixel_mode |= PMODE_BLOB;
					
				pixel_mode &= render_mode;
				
				//Alter colour based on display mode
				if(colour_mode & COLOUR_HEAT)
				{
					caddress = restrict_flt((int)( restrict_flt((float)(parts[i].temp+(-MIN_TEMP)), 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
					firea = 255;
					firer = colr = (unsigned char)color_data[caddress];
					fireg = colg = (unsigned char)color_data[caddress+1];
					fireb = colb = (unsigned char)color_data[caddress+2];
					cola = 255;
					if(pixel_mode & (FIREMODE | PMODE_GLOW)) pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
				}
				else if(colour_mode & COLOUR_LIFE)
				{
					gradv = 0.4f;
					if (!(parts[i].life<5))
						q = sqrt(parts[i].life);
					else
						q = parts[i].life;
					colr = colg = colb = sin(gradv*q) * 100 + 128;
					cola = 255;
					if(pixel_mode & (FIREMODE | PMODE_GLOW)) pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
				}
				else if (colour_mode & COLOUR_BASC)
				{
					colr = PIXR(ptypes[t].pcolors);
					colg = PIXG(ptypes[t].pcolors);
					colb = PIXB(ptypes[t].pcolors);
					pixel_mode = PMODE_FLAT;
				}
								
				//Apply decoration colour
				if(!(colour_mode & ~COLOUR_GRAD))
				{
					if(!(pixel_mode & NO_DECO) && decorations_enable)
					{
						colr = (deca*decr + (255-deca)*colr) >> 8;
						colg = (deca*decg + (255-deca)*colg) >> 8;
						colb = (deca*decb + (255-deca)*colb) >> 8;
					}
					
					if(pixel_mode & DECO_FIRE && decorations_enable)
					{
						firer = (deca*decr + (255-deca)*firer) >> 8;
						fireg = (deca*decg + (255-deca)*fireg) >> 8;
						fireb = (deca*decb + (255-deca)*fireb) >> 8;
					}
				}

				if (colour_mode & COLOUR_GRAD)
				{
					float frequency = 0.05;
					int q = parts[i].temp-40;
					colr = sin(frequency*q) * 16 + colr;
					colg = sin(frequency*q) * 16 + colg;
					colb = sin(frequency*q) * 16 + colb;
					if(pixel_mode & (FIREMODE | PMODE_GLOW)) pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
				}
				
	#ifndef OGLR
				//All colours are now set, check ranges
				if(colr>255) colr = 255;
				else if(colr<0) colr = 0;
				if(colg>255) colg = 255;
				else if(colg<0) colg = 0;
				if(colb>255) colb = 255;
				else if(colb<0) colb = 0;
				if(cola>255) cola = 255;
				else if(cola<0) cola = 0;

				if(firer>255) firer = 255;
				else if(firer<0) firer = 0;
				if(fireg>255) fireg = 255;
				else if(fireg<0) fireg = 0;
				if(fireb>255) fireb = 255;
				else if(fireb<0) fireb = 0;
				if(firea>255) firea = 255;
				else if(firea<0) firea = 0;
	#endif
					
				//Pixel rendering
				if (t==PT_SOAP)
				{
					if ((parts[i].ctype&7) == 7)
						draw_line(vid, nx, ny, (int)(parts[parts[i].tmp].x+0.5f), (int)(parts[parts[i].tmp].y+0.5f), colr, colg, colb, XRES+BARSIZE);
				}
				if(pixel_mode & PSPEC_STICKMAN)
				{
					char buff[20];  //Buffer for HP
					int s;
					int legr, legg, legb;
					playerst *cplayer;
					if(t==PT_STKM)
						cplayer = &player;
					else if(t==PT_STKM2)
						cplayer = &player2;
					else if(t==PT_FIGH)
						cplayer = &fighters[(unsigned char)parts[i].tmp];
					else
						continue;

					if (mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3)) //If mouse is in the head
					{
						sprintf(buff, "%3d", parts[i].life);  //Show HP
						drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
					}

					if (colour_mode!=COLOUR_HEAT)
					{
						if (cplayer->elem<PT_NUM)
						{
							colr = PIXR(ptypes[cplayer->elem].pcolors);
							colg = PIXG(ptypes[cplayer->elem].pcolors);
							colb = PIXB(ptypes[cplayer->elem].pcolors);
						}
						else
						{
							colr = 0x80;
							colg = 0x80;
							colb = 0xFF;
						}
					}
#ifdef OGLR
					glColor4f(((float)colr)/255.0f, ((float)colg)/255.0f, ((float)colb)/255.0f, 1.0f);
					glBegin(GL_LINE_STRIP);
					if(t==PT_FIGH)
					{
						glVertex2f(fnx, fny+2);
						glVertex2f(fnx+2, fny);
						glVertex2f(fnx, fny-2);
						glVertex2f(fnx-2, fny);
						glVertex2f(fnx, fny+2);
					}
					else
					{
						glVertex2f(fnx-2, fny-2);
						glVertex2f(fnx+2, fny-2);
						glVertex2f(fnx+2, fny+2);
						glVertex2f(fnx-2, fny+2);
						glVertex2f(fnx-2, fny-2);
					}
					glEnd();
					glBegin(GL_LINES);

					if (colour_mode!=COLOUR_HEAT)
					{
						if (t==PT_STKM2)
							glColor4f(100.0f/255.0f, 100.0f/255.0f, 1.0f, 1.0f);
						else
							glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					}

					glVertex2f(nx, ny+3);
					glVertex2f(cplayer->legs[0], cplayer->legs[1]);
					
					glVertex2f(cplayer->legs[0], cplayer->legs[1]);
					glVertex2f(cplayer->legs[4], cplayer->legs[5]);
					
					glVertex2f(nx, ny+3);
					glVertex2f(cplayer->legs[8], cplayer->legs[9]);
					
					glVertex2f(cplayer->legs[8], cplayer->legs[9]);
					glVertex2f(cplayer->legs[12], cplayer->legs[13]);
					glEnd();
#else
					s = XRES+BARSIZE;

					if (t==PT_STKM2)
					{
						legr = 100;
						legg = 100;
						legb = 255;
					}
					else
					{
						legr = 255;
						legg = 255;
						legb = 255;
					}

					if (colour_mode==COLOUR_HEAT)
					{
						legr = colr;
						legg = colg;
						legb = colb;
					}

					//head
					if(t==PT_FIGH)
					{
						draw_line(vid , nx, ny+2, nx+2, ny, colr, colg, colb, s);
						draw_line(vid , nx+2, ny, nx, ny-2, colr, colg, colb, s);
						draw_line(vid , nx, ny-2, nx-2, ny, colr, colg, colb, s);
						draw_line(vid , nx-2, ny, nx, ny+2, colr, colg, colb, s);
					}
					else
					{
						draw_line(vid , nx-2, ny+2, nx+2, ny+2, colr, colg, colb, s);
						draw_line(vid , nx-2, ny-2, nx+2, ny-2, colr, colg, colb, s);
						draw_line(vid , nx-2, ny-2, nx-2, ny+2, colr, colg, colb, s);
						draw_line(vid , nx+2, ny-2, nx+2, ny+2, colr, colg, colb, s);
					}
					//legs
					draw_line(vid , nx, ny+3, cplayer->legs[0], cplayer->legs[1], legr, legg, legb, s);
					draw_line(vid , cplayer->legs[0], cplayer->legs[1], cplayer->legs[4], cplayer->legs[5], legr, legg, legb, s);
					draw_line(vid , nx, ny+3, cplayer->legs[8], cplayer->legs[9], legr, legg, legb, s);
					draw_line(vid , cplayer->legs[8], cplayer->legs[9], cplayer->legs[12], cplayer->legs[13], legr, legg, legb, s);
#endif
				}
#ifdef OGLR
				if((display_mode & DISPLAY_EFFE) && (fabs(fnx-flx)>1.5f || fabs(fny-fly)>1.5f))
				{
					if(pixel_mode & PMODE_FLAT)
					{
						blurLineV[cblurLineV++] = nx;
						blurLineV[cblurLineV++] = ny;
						blurLineC[cblurLineC++] = ((float)colr)/255.0f;
						blurLineC[cblurLineC++] = ((float)colg)/255.0f;
						blurLineC[cblurLineC++] = ((float)colb)/255.0f;
						blurLineC[cblurLineC++] = 1.0f;
						cblurLine++;
						
						blurLineV[cblurLineV++] = flx;
						blurLineV[cblurLineV++] = fly;
						blurLineC[cblurLineC++] = ((float)colr)/255.0f;
						blurLineC[cblurLineC++] = ((float)colg)/255.0f;
						blurLineC[cblurLineC++] = ((float)colb)/255.0f;
						blurLineC[cblurLineC++] = 0.0f;
						cblurLine++;
					}
					else if(pixel_mode & PMODE_BLEND)
					{
						blurLineV[cblurLineV++] = nx;
						blurLineV[cblurLineV++] = ny;
						blurLineC[cblurLineC++] = ((float)colr)/255.0f;
						blurLineC[cblurLineC++] = ((float)colg)/255.0f;
						blurLineC[cblurLineC++] = ((float)colb)/255.0f;
						blurLineC[cblurLineC++] = ((float)cola)/255.0f;
						cblurLine++;
						
						blurLineV[cblurLineV++] = flx;
						blurLineV[cblurLineV++] = fly;
						blurLineC[cblurLineC++] = ((float)colr)/255.0f;
						blurLineC[cblurLineC++] = ((float)colg)/255.0f;
						blurLineC[cblurLineC++] = ((float)colb)/255.0f;
						blurLineC[cblurLineC++] = 0.0f;
						cblurLine++;
					}
					else if(pixel_mode & PMODE_ADD)
					{
						ablurLineV[cablurLineV++] = nx;
						ablurLineV[cablurLineV++] = ny;
						ablurLineC[cablurLineC++] = ((float)colr)/255.0f;
						ablurLineC[cablurLineC++] = ((float)colg)/255.0f;
						ablurLineC[cablurLineC++] = ((float)colb)/255.0f;
						ablurLineC[cablurLineC++] = ((float)cola)/255.0f;
						cablurLine++;
						
						ablurLineV[cablurLineV++] = flx;
						ablurLineV[cablurLineV++] = fly;
						ablurLineC[cablurLineC++] = ((float)colr)/255.0f;
						ablurLineC[cablurLineC++] = ((float)colg)/255.0f;
						ablurLineC[cablurLineC++] = ((float)colb)/255.0f;
						ablurLineC[cablurLineC++] = 0.0f;
						cablurLine++;
					}
				}
#endif
				if(pixel_mode & PMODE_FLAT)
				{
#ifdef OGLR
                    flatV[cflatV++] = nx;
                    flatV[cflatV++] = ny;
                    flatC[cflatC++] = ((float)colr)/255.0f;
                    flatC[cflatC++] = ((float)colg)/255.0f;
                    flatC[cflatC++] = ((float)colb)/255.0f;
                    flatC[cflatC++] = 1.0f;
                    cflat++;
#else
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(colr,colg,colb);
#endif
				}
				if(pixel_mode & PMODE_BLEND)
				{
#ifdef OGLR
                    flatV[cflatV++] = nx;
                    flatV[cflatV++] = ny;
                    flatC[cflatC++] = ((float)colr)/255.0f;
                    flatC[cflatC++] = ((float)colg)/255.0f;
                    flatC[cflatC++] = ((float)colb)/255.0f;
                    flatC[cflatC++] = ((float)cola)/255.0f;
                    cflat++;
#else
					blendpixel(vid, nx, ny, colr, colg, colb, cola);
#endif
				}
				if(pixel_mode & PMODE_ADD)
				{
#ifdef OGLR
                    addV[caddV++] = nx;
                    addV[caddV++] = ny;
                    addC[caddC++] = ((float)colr)/255.0f;
                    addC[caddC++] = ((float)colg)/255.0f;
                    addC[caddC++] = ((float)colb)/255.0f;
                    addC[caddC++] = ((float)cola)/255.0f;
                    cadd++;
#else
					addpixel(vid, nx, ny, colr, colg, colb, cola);
#endif
				}
				if(pixel_mode & PMODE_BLOB)
				{
#ifdef OGLR
                    blobV[cblobV++] = nx;
                    blobV[cblobV++] = ny;
                    blobC[cblobC++] = ((float)colr)/255.0f;
                    blobC[cblobC++] = ((float)colg)/255.0f;
                    blobC[cblobC++] = ((float)colb)/255.0f;
                    blobC[cblobC++] = 1.0f;
                    cblob++;
#else
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(colr,colg,colb);

					blendpixel(vid, nx+1, ny, colr, colg, colb, 223);
					blendpixel(vid, nx-1, ny, colr, colg, colb, 223);
					blendpixel(vid, nx, ny+1, colr, colg, colb, 223);
					blendpixel(vid, nx, ny-1, colr, colg, colb, 223);

					blendpixel(vid, nx+1, ny-1, colr, colg, colb, 112);
					blendpixel(vid, nx-1, ny-1, colr, colg, colb, 112);
					blendpixel(vid, nx+1, ny+1, colr, colg, colb, 112);
					blendpixel(vid, nx-1, ny+1, colr, colg, colb, 112);
#endif
				}
				if(pixel_mode & PMODE_GLOW)
				{
					int cola1 = (5*cola)/255;
#ifdef OGLR
                    glowV[cglowV++] = nx;
                    glowV[cglowV++] = ny;
                    glowC[cglowC++] = ((float)colr)/255.0f;
                    glowC[cglowC++] = ((float)colg)/255.0f;
                    glowC[cglowC++] = ((float)colb)/255.0f;
                    glowC[cglowC++] = 1.0f;
                    cglow++;
#else
					addpixel(vid, nx, ny, colr, colg, colb, (192*cola)/255);
					addpixel(vid, nx+1, ny, colr, colg, colb, (96*cola)/255);
					addpixel(vid, nx-1, ny, colr, colg, colb, (96*cola)/255);
					addpixel(vid, nx, ny+1, colr, colg, colb, (96*cola)/255);
					addpixel(vid, nx, ny-1, colr, colg, colb, (96*cola)/255);
					
					for (x = 1; x < 6; x++) {
						addpixel(vid, nx, ny-x, colr, colg, colb, cola1);
						addpixel(vid, nx, ny+x, colr, colg, colb, cola1);
						addpixel(vid, nx-x, ny, colr, colg, colb, cola1);
						addpixel(vid, nx+x, ny, colr, colg, colb, cola1);
						for (y = 1; y < 6; y++) {
							if(x + y > 7)
								continue;
							addpixel(vid, nx+x, ny-y, colr, colg, colb, cola1);
							addpixel(vid, nx-x, ny+y, colr, colg, colb, cola1);
							addpixel(vid, nx+x, ny+y, colr, colg, colb, cola1);
							addpixel(vid, nx-x, ny-y, colr, colg, colb, cola1);
						}
					}
#endif
				}
				if(pixel_mode & PMODE_BLUR)
				{
#ifdef OGLR
                    blurV[cblurV++] = nx;
                    blurV[cblurV++] = ny;
                    blurC[cblurC++] = ((float)colr)/255.0f;
                    blurC[cblurC++] = ((float)colg)/255.0f;
                    blurC[cblurC++] = ((float)colb)/255.0f;
                    blurC[cblurC++] = 1.0f;
                    cblur++;
#else
					for (x=-3; x<4; x++)
					{
						for (y=-3; y<4; y++)
						{
							if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
								blendpixel(vid, x+nx, y+ny, colr, colg, colb, 30);
							if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
								blendpixel(vid, x+nx, y+ny, colr, colg, colb, 20);
							if (abs(x)+abs(y) == 2)
								blendpixel(vid, x+nx, y+ny, colr, colg, colb, 10);
						}
					}
#endif
				}
				if(pixel_mode & PMODE_SPARK)
				{
					flicker = rand()%20;
#ifdef OGLR
					//Oh god, this is awful
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx-5;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 1.0f - ((float)flicker)/30;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx+5;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny-5;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 1.0f - ((float)flicker)/30;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny+5;
				    cline++;
#else
					gradv = 4*parts[i].life + flicker;
					for (x = 0; gradv>0.5; x++) {
						addpixel(vid, nx+x, ny, colr, colg, colb, gradv);
						addpixel(vid, nx-x, ny, colr, colg, colb, gradv);

						addpixel(vid, nx, ny+x, colr, colg, colb, gradv);
						addpixel(vid, nx, ny-x, colr, colg, colb, gradv);
						gradv = gradv/1.5f;
					}
#endif
				}
				if(pixel_mode & PMODE_FLARE)
				{
					flicker = rand()%20;
#ifdef OGLR
					//Oh god, this is awful
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx-10;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 1.0f - ((float)flicker)/40;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx+10;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny-10;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 1.0f - ((float)flicker)/30;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny+10;
				    cline++;
#else
					gradv = flicker + fabs(parts[i].vx)*17 + fabs(parts[i].vy)*17;
					blendpixel(vid, nx, ny, colr, colg, colb, (gradv*4)>255?255:(gradv*4) );
					blendpixel(vid, nx+1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(vid, nx-1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(vid, nx, ny+1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(vid, nx, ny-1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					if (gradv>255) gradv=255;
					blendpixel(vid, nx+1, ny-1, colr, colg, colb, gradv);
					blendpixel(vid, nx-1, ny-1, colr, colg, colb, gradv);
					blendpixel(vid, nx+1, ny+1, colr, colg, colb, gradv);
					blendpixel(vid, nx-1, ny+1, colr, colg, colb, gradv);
					for (x = 1; gradv>0.5; x++) {
						addpixel(vid, nx+x, ny, colr, colg, colb, gradv);
						addpixel(vid, nx-x, ny, colr, colg, colb, gradv);
						addpixel(vid, nx, ny+x, colr, colg, colb, gradv);
						addpixel(vid, nx, ny-x, colr, colg, colb, gradv);
						gradv = gradv/1.2f;
					}
#endif
				}
				if(pixel_mode & PMODE_LFLARE)
				{
					flicker = rand()%20;
#ifdef OGLR
					//Oh god, this is awful
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx-70;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 1.0f - ((float)flicker)/30;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx+70;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny-70;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 1.0f - ((float)flicker)/50;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny;
				    cline++;
				    
				    lineC[clineC++] = ((float)colr)/255.0f;
				    lineC[clineC++] = ((float)colg)/255.0f;
				    lineC[clineC++] = ((float)colb)/255.0f;
				    lineC[clineC++] = 0.0f;
				    lineV[clineV++] = fnx;
				    lineV[clineV++] = fny+70;
				    cline++;
#else
					gradv = flicker + fabs(parts[i].vx)*17 + fabs(parts[i].vy)*17;
					blendpixel(vid, nx, ny, colr, colg, colb, (gradv*4)>255?255:(gradv*4) );
					blendpixel(vid, nx+1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(vid, nx-1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(vid, nx, ny+1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(vid, nx, ny-1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					if (gradv>255) gradv=255;
					blendpixel(vid, nx+1, ny-1, colr, colg, colb, gradv);
					blendpixel(vid, nx-1, ny-1, colr, colg, colb, gradv);
					blendpixel(vid, nx+1, ny+1, colr, colg, colb, gradv);
					blendpixel(vid, nx-1, ny+1, colr, colg, colb, gradv);
					for (x = 1; gradv>0.5; x++) {
						addpixel(vid, nx+x, ny, colr, colg, colb, gradv);
						addpixel(vid, nx-x, ny, colr, colg, colb, gradv);
						addpixel(vid, nx, ny+x, colr, colg, colb, gradv);
						addpixel(vid, nx, ny-x, colr, colg, colb, gradv);
						gradv = gradv/1.01f;
					}
#endif
				}
				if (pixel_mode & EFFECT_GRAVIN)
				{
					int nxo = 0;
					int nyo = 0;
					int r;
					int fire_rv = 0;
					float drad = 0.0f;
					float ddist = 0.0f;
					orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
					for (r = 0; r < 4; r++) {
						ddist = ((float)orbd[r])/16.0f;
						drad = (M_PI * ((float)orbl[r]) / 180.0f)*1.41f;
						nxo = (int)(ddist*cos(drad));
						nyo = (int)(ddist*sin(drad));
						if (ny+nyo>0 && ny+nyo<YRES && nx+nxo>0 && nx+nxo<XRES && (pmap[ny+nyo][nx+nxo]&0xFF) != PT_PRTI)
							addpixel(vid, nx+nxo, ny+nyo, colr, colg, colb, 255-orbd[r]);
					}
				}
				if (pixel_mode & EFFECT_GRAVOUT)
				{
					int nxo = 0;
					int nyo = 0;
					int r;
					int fire_bv = 0;
					float drad = 0.0f;
					float ddist = 0.0f;
					orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
					for (r = 0; r < 4; r++) {
						ddist = ((float)orbd[r])/16.0f;
						drad = (M_PI * ((float)orbl[r]) / 180.0f)*1.41f;
						nxo = (int)(ddist*cos(drad));
						nyo = (int)(ddist*sin(drad));
						if (ny+nyo>0 && ny+nyo<YRES && nx+nxo>0 && nx+nxo<XRES && (pmap[ny+nyo][nx+nxo]&0xFF) != PT_PRTO)
							addpixel(vid, nx+nxo, ny+nyo, colr, colg, colb, 255-orbd[r]);
					}
				}
				if ((pixel_mode & EFFECT_LINES) && DEBUG_MODE)
				{
					if (mousex==(nx) && mousey==(ny))//draw lines connecting wifi/portal channels
					{
						int z;
						int type = parts[i].type;
						if (type == PT_PRTI)
							type = PT_PRTO;
						else if (type == PT_PRTO)
							type = PT_PRTI;
						for (z = 0; z<NPART; z++) {
							if (parts[z].type)
							{
								if (parts[z].type==type&&parts[z].tmp==parts[i].tmp)
									xor_line(nx,ny,(int)(parts[z].x+0.5f),(int)(parts[z].y+0.5f),vid);
							}
						}
					}
				}
				//Fire effects
				if(firea && (pixel_mode & FIRE_BLEND))
				{
#ifdef OGLR
                    smokeV[csmokeV++] = nx;
                    smokeV[csmokeV++] = ny;
                    smokeC[csmokeC++] = ((float)firer)/255.0f;
                    smokeC[csmokeC++] = ((float)fireg)/255.0f;
                    smokeC[csmokeC++] = ((float)fireb)/255.0f;
                    smokeC[csmokeC++] = ((float)firea)/255.0f;
                    csmoke++;
#else
					firea /= 2;
					fire_r[ny/CELL][nx/CELL] = (firea*firer + (255-firea)*fire_r[ny/CELL][nx/CELL]) >> 8;
					fire_g[ny/CELL][nx/CELL] = (firea*fireg + (255-firea)*fire_g[ny/CELL][nx/CELL]) >> 8;
					fire_b[ny/CELL][nx/CELL] = (firea*fireb + (255-firea)*fire_b[ny/CELL][nx/CELL]) >> 8;
#endif
				}
				if(firea && (pixel_mode & FIRE_ADD))
				{
#ifdef OGLR
                    fireV[cfireV++] = nx;
                    fireV[cfireV++] = ny;
                    fireC[cfireC++] = ((float)firer)/255.0f;
                    fireC[cfireC++] = ((float)fireg)/255.0f;
                    fireC[cfireC++] = ((float)fireb)/255.0f;
                    fireC[cfireC++] = ((float)firea)/255.0f;
                    cfire++;
#else
					firea /= 8;
					firer = ((firea*firer) >> 8) + fire_r[ny/CELL][nx/CELL];
					fireg = ((firea*fireg) >> 8) + fire_g[ny/CELL][nx/CELL];
					fireb = ((firea*fireb) >> 8) + fire_b[ny/CELL][nx/CELL];
				
					if(firer>255)
						firer = 255;
					if(fireg>255)
						fireg = 255;
					if(fireb>255)
						fireb = 255;
					
					fire_r[ny/CELL][nx/CELL] = firer;
					fire_g[ny/CELL][nx/CELL] = fireg;
					fire_b[ny/CELL][nx/CELL] = fireb;
#endif
				}
			}
		}
	}
#ifdef OGLR		
        
        //Go into array mode
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      

		if(cablurLine)
		{
			// -- BEGIN LINES -- //
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable( GL_LINE_SMOOTH );
			glColorPointer(4, GL_FLOAT, 0, &ablurLineC[0]);
			glVertexPointer(2, GL_FLOAT, 0, &ablurLineV[0]);
			
			glDrawArrays(GL_LINES, 0, cablurLine);
			
			//Clear some stuff we set
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_LINE_SMOOTH);
			// -- END LINES -- //
		}
		if(cblurLine)
		{
			// -- BEGIN LINES -- //
			glEnable( GL_LINE_SMOOTH );
			glColorPointer(4, GL_FLOAT, 0, &blurLineC[0]);
			glVertexPointer(2, GL_FLOAT, 0, &blurLineV[0]);
			
			glDrawArrays(GL_LINES, 0, cblurLine);
			
			//Clear some stuff we set
			glDisable(GL_LINE_SMOOTH);
			// -- END LINES -- //
		}
	
 		if(cflat)
		{
			// -- BEGIN FLAT -- //
			//Set point size (size of fire texture)
			glPointSize(1.0f);

		    glColorPointer(4, GL_FLOAT, 0, &flatC[0]);
		    glVertexPointer(2, GL_INT, 0, &flatV[0]);

		    glDrawArrays(GL_POINTS, 0, cflat);
		    
		    //Clear some stuff we set
		    // -- END FLAT -- //
        }
        
        if(cblob)
		{
		    // -- BEGIN BLOB -- //
			glEnable( GL_POINT_SMOOTH ); //Blobs!
			glPointSize(2.5f);

		    glColorPointer(4, GL_FLOAT, 0, &blobC[0]);
		    glVertexPointer(2, GL_INT, 0, &blobV[0]);

		    glDrawArrays(GL_POINTS, 0, cblob);
		    
		    //Clear some stuff we set
		    glDisable( GL_POINT_SMOOTH );
		    // -- END BLOB -- //
		}
        
        if(cglow || cblur)
        {
        	// -- BEGIN GLOW -- //
			//Start and prepare fire program
			glEnable(GL_TEXTURE_2D);
			glUseProgram(fireProg);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glowAlpha);
			glUniform1i(glGetUniformLocation(fireProg, "fireAlpha"), 0);
			
			glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
		
			//Make sure we can use texture coords on points
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		
			//Set point size (size of fire texture)
			glPointSize(11.0f);
			
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			if(cglow)
			{
				glColorPointer(4, GL_FLOAT, 0, &glowC[0]);
				glVertexPointer(2, GL_INT, 0, &glowV[0]);
	
				glDrawArrays(GL_POINTS, 0, cglow);
			}
			
			glPointSize(7.0f);
			
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			if(cblur)
			{
				glBindTexture(GL_TEXTURE_2D, blurAlpha);
			
				glColorPointer(4, GL_FLOAT, 0, &blurC[0]);
				glVertexPointer(2, GL_INT, 0, &blurV[0]);

				glDrawArrays(GL_POINTS, 0, cblur);
			}
		    
		    //Clear some stuff we set
		    glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		    // -- END GLOW -- //
        }
        
 		if(cadd)
		{
			// -- BEGIN ADD -- //
			//Set point size (size of fire texture)
			glPointSize(1.0f);

		    glColorPointer(4, GL_FLOAT, 0, &addC[0]);
		    glVertexPointer(2, GL_INT, 0, &addV[0]);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		    glDrawArrays(GL_POINTS, 0, cadd);
		    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		    //Clear some stuff we set
		    // -- END ADD -- //
        }
        
        if(cline)
		{
			// -- BEGIN LINES -- //
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable( GL_LINE_SMOOTH );
		    glColorPointer(4, GL_FLOAT, 0, &lineC[0]);
		    glVertexPointer(2, GL_FLOAT, 0, &lineV[0]);

		    glDrawArrays(GL_LINE_STRIP, 0, cline);
		    
		    //Clear some stuff we set
		    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		    glDisable(GL_LINE_SMOOTH);
		    // -- END LINES -- //
        }
		
		if(cfire || csmoke)
		{
			// -- BEGIN FIRE -- //
			//Start and prepare fire program
			glEnable(GL_TEXTURE_2D);
			glUseProgram(fireProg);
			//glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fireAlpha);
			glUniform1i(glGetUniformLocation(fireProg, "fireAlpha"), 0);
		
			//Make sure we can use texture coords on points
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		
			//Set point size (size of fire texture)
			glPointSize(CELL*3);
			
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			if(cfire)
			{
				glColorPointer(4, GL_FLOAT, 0, &fireC[0]);
				glVertexPointer(2, GL_INT, 0, &fireV[0]);

				glDrawArrays(GL_POINTS, 0, cfire);
			}
			
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			if(csmoke)
			{
				glColorPointer(4, GL_FLOAT, 0, &smokeC[0]);
				glVertexPointer(2, GL_INT, 0, &smokeV[0]);

				glDrawArrays(GL_POINTS, 0, csmoke);
			}
		    
		    //Clear some stuff we set
		    glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		    // -- END FIRE -- //
        }

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);     
        
        //Reset FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        //Drawing the FBO onto the screen sounds like a cool idea now
        
        glBlendFunc(origBlendSrc, origBlendDst);
#endif
}

#ifdef OGLR
void draw_parts_fbo()
{
	glEnable( GL_TEXTURE_2D );
	if(display_mode & DISPLAY_WARP)
	{
		float xres = XRES, yres = YRES;
		glUseProgram(lensProg);
		glActiveTexture(GL_TEXTURE0);			
		glBindTexture(GL_TEXTURE_2D, partsFboTex);
		glUniform1i(glGetUniformLocation(lensProg, "pTex"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, partsTFX);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_RED, GL_FLOAT, gravx);
		glUniform1i(glGetUniformLocation(lensProg, "tfX"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, partsTFY);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_GREEN, GL_FLOAT, gravy);
		glUniform1i(glGetUniformLocation(lensProg, "tfY"), 2);
		glActiveTexture(GL_TEXTURE0);
		glUniform1fv(glGetUniformLocation(lensProg, "xres"), 1, &xres);
		glUniform1fv(glGetUniformLocation(lensProg, "yres"), 1, &yres);
	}
	else
	{	
		glBindTexture(GL_TEXTURE_2D, partsFboTex);
		glBlendFunc(GL_ONE, GL_ONE);
	}
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(1, 0);
	glVertex3f(XRES*sdl_scale, (YRES+MENUSIZE)*sdl_scale, 1.0);
	glTexCoord2d(0, 0);
	glVertex3f(0, (YRES+MENUSIZE)*sdl_scale, 1.0);
	glTexCoord2d(0, 1);
	glVertex3f(0, MENUSIZE*sdl_scale, 1.0);
	glTexCoord2d(1, 1);
	glVertex3f(XRES*sdl_scale, MENUSIZE*sdl_scale, 1.0);
	glEnd();
	
	if(display_mode & DISPLAY_WARP)
	{
		glUseProgram(0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable( GL_TEXTURE_2D );
}
#endif

// draw the graphics that appear before update_particles is called
void render_before(pixel *part_vbuf)
{
#ifdef OGLR
		if (display_mode & DISPLAY_PERS)//save background for persistent, then clear
		{
			clearScreen(0.01f);
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		}
		else //clear screen every frame
		{
			clearScreen(1.0f);
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
			if (display_mode & DISPLAY_AIR)//air only gets drawn in these modes
			{
				draw_air(part_vbuf);
			}
		}
#else
		if (display_mode & DISPLAY_AIR)//air only gets drawn in these modes
		{
			draw_air(part_vbuf);
		}
		else if (display_mode & DISPLAY_PERS)//save background for persistent, then clear
		{
			memcpy(part_vbuf, pers_bg, (XRES+BARSIZE)*YRES*PIXELSIZE);
			memset(part_vbuf+((XRES+BARSIZE)*YRES), 0, ((XRES+BARSIZE)*YRES*PIXELSIZE)-((XRES+BARSIZE)*YRES*PIXELSIZE));
		}
		else //clear screen every frame
		{
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		}
#endif
		if(ngrav_enable && drawgrav_enable)
			draw_grav(part_vbuf);
		draw_walls(part_vbuf);
}

int persist_counter = 0;
// draw the graphics that appear after update_particles is called
void render_after(pixel *part_vbuf, pixel *vid_buf)
{
	render_parts(part_vbuf); //draw particles
	draw_other(part_vbuf);
	//if(su == WL_GRAV+100)
	//	draw_grav_zones(part_vbuf);
	if (vid_buf && (display_mode & DISPLAY_PERS))
	{
		if (!persist_counter)
		{
			dim_copy_pers(pers_bg, vid_buf);
		}
		else
		{
			memcpy(pers_bg, vid_buf, (XRES+BARSIZE)*YRES*PIXELSIZE);
		}
		persist_counter = (persist_counter+1) % 3;
	}
#ifndef OGLR
	if (render_mode & FIREMODE)
		render_fire(part_vbuf);
#endif

	render_signs(part_vbuf);

#ifndef OGLR
	if(vid_buf && ngrav_enable && (display_mode & DISPLAY_WARP))
		render_gravlensing(part_vbuf, vid_buf);
#endif
}

void draw_walls(pixel *vid)
{
	int x, y, i, j, cr, cg, cb, nx, ny, t;
	unsigned char wt;
	float lx, ly;
	pixel pc;
	pixel gc;
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
			if (bmap[y][x])
			{
				wt = bmap[y][x]-UI_ACTUALSTART;
				if (wt<0 || wt>=UI_WALLCOUNT)
					continue;
				pc = wtypes[wt].colour;
				gc = wtypes[wt].eglow;

				// standard wall patterns
				if (wtypes[wt].drawstyle==1)
				{
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
				}
				else if (wtypes[wt].drawstyle==2)
				{
					for (j=0; j<CELL; j+=2)
						for (i=0; i<CELL; i+=2)
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
				}
				else if (wtypes[wt].drawstyle==3)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
				}
				else if (wtypes[wt].drawstyle==4)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
							if(i == j)
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
							else if  (i == j+1 || (i == 0 && j == CELL-1))
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = gc;
							else 
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x202020);
				}

				// special rendering for some walls
				if (bmap[y][x]==WL_EWALL)
				{
					if (emap[y][x])
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if (i&j&1)
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
					}
					else
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if (!(i&j&1))
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
					}
				}
				else if (bmap[y][x]==WL_WALLELEC)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = pc;
							else
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
						}
				}
				else if (bmap[y][x]==WL_EHOLE)
				{
					if (emap[y][x])
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
						for (j=0; j<CELL; j+=2)
							for (i=0; i<CELL; i+=2)
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x000000);
					}
					else
					{
						for (j=0; j<CELL; j+=2)
							for (i=0; i<CELL; i+=2)
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
					}
				}
				if (render_mode & PMODE_BLOB)
				{
					// when in blob view, draw some blobs...
					if (wtypes[wt].drawstyle==1)
					{
						for (j=0; j<CELL; j+=2)
							for (i=(j>>1)&1; i<CELL; i+=2)
								drawblob(vid, (x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
					}
					else if (wtypes[wt].drawstyle==2)
					{
						for (j=0; j<CELL; j+=2)
							for (i=0; i<CELL; i+=2)
								drawblob(vid, (x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
					}
					else if (wtypes[wt].drawstyle==3)
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								drawblob(vid, (x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
					}
					else if (wtypes[wt].drawstyle==4)
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if(i == j)
									drawblob(vid, (x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
								else if  (i == j+1 || (i == 0 && j == CELL-1))
									drawblob(vid, (x*CELL+i), (y*CELL+j), PIXR(gc), PIXG(gc), PIXB(gc));
								else 
									drawblob(vid, (x*CELL+i), (y*CELL+j), 0x20, 0x20, 0x20);
					}
					if (bmap[y][x]==WL_EWALL)
					{
						if (emap[y][x])
						{
							for (j=0; j<CELL; j++)
								for (i=0; i<CELL; i++)
									if (i&j&1)
										drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
						}
						else
						{
							for (j=0; j<CELL; j++)
								for (i=0; i<CELL; i++)
									if (!(i&j&1))
										drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
						}
					}
					else if (bmap[y][x]==WL_WALLELEC)
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
							{
								if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
									drawblob(vid, (x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
								else
									drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
							}
					}
					else if (bmap[y][x]==WL_EHOLE)
					{
						if (emap[y][x])
						{
							for (j=0; j<CELL; j++)
								for (i=0; i<CELL; i++)
									drawblob(vid, (x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
							for (j=0; j<CELL; j+=2)
								for (i=0; i<CELL; i+=2)
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x000000);
						}
						else
						{
							for (j=0; j<CELL; j+=2)
								for (i=0; i<CELL; i+=2)
									drawblob(vid, (x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
						}
					}
				}
				if (wtypes[wt].eglow && emap[y][x])
				{
					// glow if electrified
					pc = wtypes[wt].eglow;
					cr = fire_r[y][x] + PIXR(pc);
					if (cr > 255) cr = 255;
					fire_r[y][x] = cr;
					cg = fire_g[y][x] + PIXG(pc);
					if (cg > 255) cg = 255;
					fire_g[y][x] = cg;
					cb = fire_b[y][x] + PIXB(pc);
					if (cb > 255) cb = 255;
					fire_b[y][x] = cb;
					
				}
			}

	// draw streamlines
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
			if (bmap[y][x]==WL_STREAM)
			{
				lx = x*CELL + CELL*0.5f;
				ly = y*CELL + CELL*0.5f;
				for (t=0; t<1024; t++)
				{
					nx = (int)(lx+0.5f);
					ny = (int)(ly+0.5f);
					if (nx<0 || nx>=XRES || ny<0 || ny>=YRES)
						break;
					addpixel(vid, nx, ny, 255, 255, 255, 64);
					i = nx/CELL;
					j = ny/CELL;
					lx += vx[j][i]*0.125f;
					ly += vy[j][i]*0.125f;
					if (bmap[j][i]==WL_STREAM && i!=x && j!=y)
						break;
				}
				drawtext(vid, x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
			}
}

void create_decorations(int x, int y, int rx, int ry, int r, int g, int b, int click, int tool)
{
	int i,j,rp;
	if (rx==0 && ry==0)
	{
		create_decoration(x,y,r,g,b,click,tool);
		return;
	}
	for (j=-ry; j<=ry; j++)
		for (i=-rx; i<=rx; i++)
			if(y+j>=0 && x+i>=0 && x+i<XRES && y+j<YRES)
				if (InCurrentBrush(i, j, rx, ry)){
					create_decoration(x+i,y+j,r,g,b,click,tool);
				}
}
void create_decoration(int x, int y, int r, int g, int b, int click, int tool)
{
	int rp, tr = 0, tg = 0, tb = 0;
	rp = pmap[y][x];
	if (!rp)
		return;
	if (tool == DECO_DRAW)
	{
		if (click == 4)
			parts[rp>>8].dcolour = 0;
		else
			parts[rp>>8].dcolour = ((255<<24)|(r<<16)|(g<<8)|b);
	}
	else if (tool == DECO_LIGHTEN)
	{//maybe get a better lighten/darken?
		if (parts[rp>>8].dcolour == 0)
			return;
		tr = (parts[rp>>8].dcolour>>16)&0xFF;
		tg = (parts[rp>>8].dcolour>>8)&0xFF;
		tb = (parts[rp>>8].dcolour)&0xFF;
		parts[rp>>8].dcolour = ((parts[rp>>8].dcolour&0xFF000000)|(clamp_flt(tr+(255-tr)*0.02+1, 0,255)<<16)|(clamp_flt(tg+(255-tg)*0.02+1, 0,255)<<8)|clamp_flt(tb+(255-tb)*0.02+1, 0,255));
	}
	else if (tool == DECO_DARKEN)
	{
		if (parts[rp>>8].dcolour == 0)
			return;
		tr = (parts[rp>>8].dcolour>>16)&0xFF;
		tg = (parts[rp>>8].dcolour>>8)&0xFF;
		tb = (parts[rp>>8].dcolour)&0xFF;
		parts[rp>>8].dcolour = ((parts[rp>>8].dcolour&0xFF000000)|(clamp_flt(tr-(tr)*0.02, 0,255)<<16)|(clamp_flt(tg-(tg)*0.02, 0,255)<<8)|clamp_flt(tb-(tb)*0.02, 0,255));
	}
	else if (tool == DECO_SMUDGE)
	{
		int rx, ry, num = 0, ta = 0;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
			{
				if ((pmap[y+ry][x+rx]&0xFF) && parts[pmap[y+ry][x+rx]>>8].dcolour)
				{
					num++;
					ta += (parts[pmap[y+ry][x+rx]>>8].dcolour>>24)&0xFF;
					tr += (parts[pmap[y+ry][x+rx]>>8].dcolour>>16)&0xFF;
					tg += (parts[pmap[y+ry][x+rx]>>8].dcolour>>8)&0xFF;
					tb += (parts[pmap[y+ry][x+rx]>>8].dcolour)&0xFF;
				}
			}
		if (num == 0)
			return;
		ta = fminf(255,(int)((float)ta/num+.5));
		tr = fminf(255,(int)((float)tr/num+.5));
		tg = fminf(255,(int)((float)tg/num+.5));
		tb = fminf(255,(int)((float)tb/num+.5));
		if (!parts[rp>>8].dcolour)
			ta = fmaxf(0,ta-3);
		parts[rp>>8].dcolour = ((ta<<24)|(tr<<16)|(tg<<8)|tb);
	}
}
void line_decorations(int x1, int y1, int x2, int y2, int rx, int ry, int r, int g, int b, int click, int tool)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (cp)
			create_decorations(y, x, rx, ry, r, g, b, click, tool);
		else
			create_decorations(x, y, rx, ry, r, g, b, click, tool);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry))
			{
				if (cp)
					create_decorations(y, x, rx, ry, r, g, b, click, tool);
				else
					create_decorations(x, y, rx, ry, r, g, b, click, tool);
			}
			e -= 1.0f;
		}
	}
}
void box_decorations(int x1, int y1, int x2, int y2, int r, int g, int b, int click, int tool)
{
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			create_decorations(i, j, 0, 0, r, g, b, click, tool);
}

//draws the photon colors in the HUD
void draw_wavelengths(pixel *vid, int x, int y, int h, int wl)
{
	int i,cr,cg,cb,j;
	int tmp;
	fillrect(vid,x-1,y-1,30+1,h+1,64,64,64,255); // coords -1 size +1 to work around bug in fillrect - TODO: fix fillrect
	for (i=0; i<30; i++)
	{
		if ((wl>>i)&1)
		{
			// Need a spread of wavelengths to get a smooth spectrum, 5 bits seems to work reasonably well
			if (i>2) tmp = 0x1F << (i-2);
			else tmp = 0x1F >> (2-i);
			cg = 0;
			cb = 0;
			cr = 0;
			for (j=0; j<12; j++) {
				cr += (tmp >> (j+18)) & 1;
				cb += (tmp >>  j)     & 1;
			}
			for (j=0; j<13; j++)
				cg += (tmp >> (j+9))  & 1;
			tmp = 624/(cr+cg+cb+1);
			cr *= tmp;
			cg *= tmp;
			cb *= tmp;
			for (j=0; j<h; j++) blendpixel(vid,x+29-i,y+j,cr>255?255:cr,cg>255?255:cg,cb>255?255:cb,255);
		}
	}
}

void render_signs(pixel *vid_buf)
{
	int i, j, x, y, w, h, dx, dy,mx,my,b=1,bq;
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0])
		{
			char buff[256];  //Buffer
			get_sign_pos(i, &x, &y, &w, &h);
			clearrect(vid_buf, x, y, w, h);
			drawrect(vid_buf, x, y, w, h, 192, 192, 192, 255);

			//Displaying special information
			if (strcmp(signs[i].text, "{p}")==0)
			{
				float pressure = 0.0f;
				if (signs[i].x>=0 && signs[i].x<XRES && signs[i].y>=0 && signs[i].y<YRES)
					pressure = pv[signs[i].y/CELL][signs[i].x/CELL];
				sprintf(buff, "Pressure: %3.2f", pressure);  //...pressure
				drawtext(vid_buf, x+3, y+3, buff, 255, 255, 255, 255);
			}
			if (strcmp(signs[i].text, "{t}")==0)
			{
				if (signs[i].x>=0 && signs[i].x<XRES && signs[i].y>=0 && signs[i].y<YRES && pmap[signs[i].y][signs[i].x])
					sprintf(buff, "Temp: %4.2f", parts[pmap[signs[i].y][signs[i].x]>>8].temp-273.15);  //...temperature
				else
					sprintf(buff, "Temp: 0.00");  //...temperature
				drawtext(vid_buf, x+3, y+3, buff, 255, 255, 255, 255);
			}

			if (sregexp(signs[i].text, "^{c:[0-9]*|.*}$")==0)
			{
				int sldr, startm;
				memset(buff, 0, sizeof(buff));
				for (sldr=3; signs[i].text[sldr-1] != '|'; sldr++)
					startm = sldr + 1;
				sldr = startm;
				while (signs[i].text[sldr] != '}')
				{
					buff[sldr - startm] = signs[i].text[sldr];
					sldr++;
				}
				drawtext(vid_buf, x+3, y+3, buff, 0, 191, 255, 255);
			}

			//Usual text
			if (strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}") && sregexp(signs[i].text, "^{c:[0-9]*|.*}$"))
				drawtext(vid_buf, x+3, y+3, signs[i].text, 255, 255, 255, 255);

			x = signs[i].x;
			y = signs[i].y;
			dx = 1 - signs[i].ju;
			dy = (signs[i].y > 18) ? -1 : 1;
			for (j=0; j<4; j++)
			{
				drawpixel(vid_buf, x, y, 192, 192, 192, 255);
				x+=dx;
				y+=dy;
			}
			if (MSIGN==i)
			{
				bq = b;
				mouse_get_state(&mx, &my);
				mouse_coords_window_to_sim(&mx, &my, mx, my);
				signs[i].x = mx;
				signs[i].y = my;
			}
		}
}

void render_gravlensing(pixel *src, pixel * dst)
{
	int nx, ny, rx, ry, gx, gy, bx, by, co;
	int r, g, b;
	pixel t;
	for(nx = 0; nx < XRES; nx++)
	{
		for(ny = 0; ny < YRES; ny++)
		{
			co = (ny/CELL)*(XRES/CELL)+(nx/CELL);
			rx = (int)(nx-gravx[co]*0.75f+0.5f);
			ry = (int)(ny-gravy[co]*0.75f+0.5f);
			gx = (int)(nx-gravx[co]*0.875f+0.5f);
			gy = (int)(ny-gravy[co]*0.875f+0.5f);
			bx = (int)(nx-gravx[co]+0.5f);
			by = (int)(ny-gravy[co]+0.5f);
			if(rx > 0 && rx < XRES && ry > 0 && ry < YRES && gx > 0 && gx < XRES && gy > 0 && gy < YRES && bx > 0 && bx < XRES && by > 0 && by < YRES)
			{
				t = dst[ny*(XRES+BARSIZE)+nx];
				r = PIXR(src[ry*(XRES+BARSIZE)+rx]) + PIXR(t);
				g = PIXG(src[gy*(XRES+BARSIZE)+gx]) + PIXG(t);
				b = PIXB(src[by*(XRES+BARSIZE)+bx]) + PIXB(t);
				if (r>255)
					r = 255;
				if (g>255)
					g = 255;
				if (b>255)
					b = 255;
				dst[ny*(XRES+BARSIZE)+nx] = PIXRGB(r,g,b);
				//	addpixel(dst, nx, ny, PIXR(src[ry*(XRES+BARSIZE)+rx]), PIXG(src[gy*(XRES+BARSIZE)+gx]), PIXB(src[by*(XRES+BARSIZE)+bx]), 255);
			}

			/*rx = nx+(gravxf[(ny*XRES)+nx]*0.5f);
			ry = ny+(gravyf[(ny*XRES)+nx]*0.5f);
			gx = nx+(gravxf[(ny*XRES)+nx]*0.75f);
			gy = ny+(gravyf[(ny*XRES)+nx]*0.75f);
			bx = nx+(gravxf[(ny*XRES)+nx]);
			by = ny+(gravyf[(ny*XRES)+nx]);
			if(rx > 0 && rx < XRES && ry > 0 && ry < YRES && gravp[ny/CELL][nx/CELL]*0.5f > -8.0f)
				addpixel(dst, rx, ry, PIXR(src[ry*(XRES+BARSIZE)+rx]), 0, 0, 255);
			if(gx > 0 && gx < XRES && gy > 0 && gy < YRES && gravp[ny/CELL][nx/CELL]*0.75f > -8.0f)
				addpixel(dst, gx, gy, 0, PIXG(src[ry*(XRES+BARSIZE)+rx]), 0, 255);
			if(bx > 0 && bx < XRES && by > 0 && by < YRES && gravp[ny/CELL][nx/CELL] > -8.0f)
				addpixel(dst, bx, by, 0, 0, PIXB(src[ry*(XRES+BARSIZE)+rx]), 255);*/
		}
	}
}

void render_fire(pixel *vid)
{
	int i,j,x,y,r,g,b,nx,ny;
	for (j=0; j<YRES/CELL; j++)
		for (i=0; i<XRES/CELL; i++)
		{
			r = fire_r[j][i];
			g = fire_g[j][i];
			b = fire_b[j][i];
			if (r || g || b)
				for (y=-CELL; y<2*CELL; y++)
					for (x=-CELL; x<2*CELL; x++)
						addpixel(vid, i*CELL+x, j*CELL+y, r, g, b, fire_alpha[y+CELL][x+CELL]);
			r *= 8;
			g *= 8;
			b *= 8;
			for (y=-1; y<2; y++)
				for (x=-1; x<2; x++)
					if ((x || y) && i+x>=0 && j+y>=0 && i+x<XRES/CELL && j+y<YRES/CELL)
					{
						r += fire_r[j+y][i+x];
						g += fire_g[j+y][i+x];
						b += fire_b[j+y][i+x];
					}
			r /= 16;
			g /= 16;
			b /= 16;
			fire_r[j][i] = r>4 ? r-4 : 0;
			fire_g[j][i] = g>4 ? g-4 : 0;
			fire_b[j][i] = b>4 ? b-4 : 0;
		}
}

void prepare_alpha(int size, float intensity)
{
	//TODO: implement size
	int x,y,i,j,c;
	float multiplier = 255.0f*intensity;
	float temp[CELL*3][CELL*3];
	float fire_alphaf[CELL*3][CELL*3];
	float glow_alphaf[11][11];
	float blur_alphaf[7][7];
	memset(temp, 0, sizeof(temp));
	for (x=0; x<CELL; x++)
		for (y=0; y<CELL; y++)
			for (i=-CELL; i<CELL; i++)
				for (j=-CELL; j<CELL; j++)
					temp[y+CELL+j][x+CELL+i] += expf(-0.1f*(i*i+j*j));
	for (x=0; x<CELL*3; x++)
		for (y=0; y<CELL*3; y++)
			fire_alpha[y][x] = (int)(multiplier*temp[y][x]/(CELL*CELL));
			
#ifdef OGLR
	for (x=0; x<CELL*3; x++)
		for (y=0; y<CELL*3; y++)
		{
			fire_alphaf[y][x] = (intensity*temp[y][x]/((float)(CELL*CELL)))/2.0f;
		}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fireAlpha);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CELL*3, CELL*3, GL_ALPHA, GL_FLOAT, fire_alphaf);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	
	memset(glow_alphaf, 0, sizeof(glow_alphaf));
	
	c = 5;
	
	glow_alphaf[c][c] = 0.8f;
	glow_alphaf[c][c-1] = 0.4f;
	glow_alphaf[c][c+1] = 0.4f;
	glow_alphaf[c-1][c] = 0.4f;
	glow_alphaf[c+1][c] = 0.4f;
	for (x = 1; x < 6; x++) {
		glow_alphaf[c][c-x] += 0.02f;
		glow_alphaf[c][c+x] += 0.02f;
		glow_alphaf[c-x][c] += 0.02f;
		glow_alphaf[c+x][c] += 0.02f;
		for (y = 1; y < 6; y++) {
			if(x + y > 7)
				continue;
			glow_alphaf[c+x][c-y] += 0.02f;
			glow_alphaf[c-x][c+y] += 0.02f;
			glow_alphaf[c+x][c+y] += 0.02f;
			glow_alphaf[c-x][c-y] += 0.02f;
		}
	}
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, glowAlpha);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 11, 11, GL_ALPHA, GL_FLOAT, glow_alphaf);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	
	c = 3;
	
	for (x=-3; x<4; x++)
	{
		for (y=-3; y<4; y++)
		{
			if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
				blur_alphaf[c+x][c-y] = 0.11f;
			if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
				blur_alphaf[c+x][c-y] = 0.08f;
			if (abs(x)+abs(y) == 2)
				blur_alphaf[c+x][c-y] = 0.04f;
		}
	}
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, blurAlpha);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 7, 7, GL_ALPHA, GL_FLOAT, blur_alphaf);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
#endif
}

pixel *render_packed_rgb(void *image, int width, int height, int cmp_size)
{
	unsigned char *tmp;
	pixel *res;
	int i;

	tmp = malloc(width*height*3);
	if (!tmp)
		return NULL;
	res = malloc(width*height*PIXELSIZE);
	if (!res)
	{
		free(tmp);
		return NULL;
	}

	i = width*height*3;
	if (BZ2_bzBuffToBuffDecompress((char *)tmp, (unsigned *)&i, (char *)image, cmp_size, 0, 0))
	{
		free(res);
		free(tmp);
		return NULL;
	}

	for (i=0; i<width*height; i++)
		res[i] = PIXRGB(tmp[3*i], tmp[3*i+1], tmp[3*i+2]);

	free(tmp);
	return res;
}

void draw_rgba_image(pixel *vid, unsigned char *data, int x, int y, float alpha)
{
	unsigned char w, h;
	int i, j;
	unsigned char r, g, b, a;
	if (!data) return;
	w = *(data++)&0xFF;
	h = *(data++)&0xFF;
	for (j=0; j<h; j++)
	{
		for (i=0; i<w; i++)
		{
			r = *(data++)&0xFF;
			g = *(data++)&0xFF;
			b = *(data++)&0xFF;
			a = *(data++)&0xFF;
			drawpixel(vid, x+i, y+j, r, g, b, a*alpha);
		}
	}
}

void draw_image(pixel *vid, pixel *img, int x, int y, int w, int h, int a)
{
	int i, j, r, g, b;
	if (!img) return;
	for (j=0; j<h; j++)
		for (i=0; i<w; i++)
		{
			r = PIXR(*img);
			g = PIXG(*img);
			b = PIXB(*img);
			drawpixel(vid, x+i, y+j, r, g, b, a);
			img++;
		}
}

void dim_copy(pixel *dst, pixel *src) //old persistent, unused
{
	int i,r,g,b;
	for (i=0; i<XRES*YRES; i++)
	{
		r = PIXR(src[i]);
		g = PIXG(src[i]);
		b = PIXB(src[i]);
		if (r>0)
			r--;
		if (g>0)
			g--;
		if (b>0)
			b--;
		dst[i] = PIXRGB(r,g,b);
	}
}

void dim_copy_pers(pixel *dst, pixel *src) //for persistent view, reduces rgb slowly
{
	int i,r,g,b;
	for (i=0; i<(XRES+BARSIZE)*YRES; i++)
	{
		r = PIXR(src[i]);
		g = PIXG(src[i]);
		b = PIXB(src[i]);
		if (r>0)
			r--;
		if (g>0)
			g--;
		if (b>0)
			b--;
		dst[i] = PIXRGB(r,g,b);
	}
}

void render_zoom(pixel *img) //draws the zoom box
{
#ifdef OGLR
	int origBlendSrc, origBlendDst;
	float zcx1, zcx0, zcy1, zcy0, yfactor, xfactor, i; //X-Factor is shit, btw
	xfactor = 1.0f/(float)XRES;
	yfactor = 1.0f/(float)YRES;

	zcx0 = (zoom_x)*xfactor;
	zcx1 = (zoom_x+ZSIZE)*xfactor;
	zcy0 = (zoom_y)*yfactor;
	zcy1 = ((zoom_y+ZSIZE))*yfactor;

	glGetIntegerv(GL_BLEND_SRC, &origBlendSrc);
	glGetIntegerv(GL_BLEND_DST, &origBlendDst);
	glBlendFunc(GL_ONE, GL_ZERO);

	glEnable( GL_TEXTURE_2D );
	//glReadBuffer(GL_AUX0);
	glBindTexture(GL_TEXTURE_2D, partsFboTex);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(zcx1, zcy1);
	glVertex3f((zoom_wx+ZSIZE*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR))*sdl_scale, 1.0);
	glTexCoord2d(zcx0, zcy1);
	glVertex3f(zoom_wx*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR))*sdl_scale, 1.0);
	glTexCoord2d(zcx0, zcy0);
	glVertex3f(zoom_wx*sdl_scale, (YRES+MENUSIZE-zoom_wy)*sdl_scale, 1.0);
	glTexCoord2d(zcx1, zcy0);
	glVertex3f((zoom_wx+ZSIZE*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-zoom_wy)*sdl_scale, 1.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable( GL_TEXTURE_2D );
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glLineWidth(sdl_scale);
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINES);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	for(i = 0; i < ZSIZE; i++)
	{
		glVertex2f((zoom_wx+ZSIZE*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR)+i*ZFACTOR)*sdl_scale);
		glVertex2f(zoom_wx*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR)+i*ZFACTOR)*sdl_scale);
		glVertex2f((zoom_wx+i*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR))*sdl_scale);
		glVertex2f((zoom_wx+i*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-zoom_wy)*sdl_scale);
	}
	glEnd();
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	glVertex3i((zoom_wx-1)*sdl_scale, (YRES+MENUSIZE-zoom_wy)*sdl_scale, 0);
	glVertex3i((zoom_wx-1)*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR))*sdl_scale, 0);
	glVertex3i((zoom_wx+ZSIZE*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-(zoom_wy+ZSIZE*ZFACTOR))*sdl_scale, 0);
	glVertex3i((zoom_wx+ZSIZE*ZFACTOR)*sdl_scale, (YRES+MENUSIZE-zoom_wy)*sdl_scale, 0);
	glVertex3i((zoom_wx-1)*sdl_scale, (YRES+MENUSIZE-zoom_wy)*sdl_scale, 0);
	glEnd();
	glDisable(GL_LINE_SMOOTH);
	
	glDisable(GL_LINE_SMOOTH);

	if(zoom_en)
	{	
		glEnable(GL_COLOR_LOGIC_OP);
		//glEnable(GL_LINE_SMOOTH);
		glLogicOp(GL_XOR);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_STRIP);
		glVertex3i((zoom_x-1)*sdl_scale, (YRES+MENUSIZE-(zoom_y-1))*sdl_scale, 0);
		glVertex3i((zoom_x-1)*sdl_scale, (YRES+MENUSIZE-(zoom_y+ZSIZE))*sdl_scale, 0);
		glVertex3i((zoom_x+ZSIZE)*sdl_scale, (YRES+MENUSIZE-(zoom_y+ZSIZE))*sdl_scale, 0);
		glVertex3i((zoom_x+ZSIZE)*sdl_scale, (YRES+MENUSIZE-(zoom_y-1))*sdl_scale, 0);
		glVertex3i((zoom_x-1)*sdl_scale, (YRES+MENUSIZE-(zoom_y-1))*sdl_scale, 0);
		glEnd();
		glDisable(GL_COLOR_LOGIC_OP);
	}
	glLineWidth(1);
	glBlendFunc(origBlendSrc, origBlendDst);
#else
	int x, y, i, j;
	pixel pix;
	drawrect(img, zoom_wx-2, zoom_wy-2, ZSIZE*ZFACTOR+2, ZSIZE*ZFACTOR+2, 192, 192, 192, 255);
	drawrect(img, zoom_wx-1, zoom_wy-1, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR, 0, 0, 0, 255);
	clearrect(img, zoom_wx, zoom_wy, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR);
	for (j=0; j<ZSIZE; j++)
		for (i=0; i<ZSIZE; i++)
		{
			pix = img[(j+zoom_y)*(XRES+BARSIZE)+(i+zoom_x)];
			for (y=0; y<ZFACTOR-1; y++)
				for (x=0; x<ZFACTOR-1; x++)
					img[(j*ZFACTOR+y+zoom_wy)*(XRES+BARSIZE)+(i*ZFACTOR+x+zoom_wx)] = pix;
		}
	if (zoom_en)
	{
		for (j=-1; j<=ZSIZE; j++)
		{
			xor_pixel(zoom_x+j, zoom_y-1, img);
			xor_pixel(zoom_x+j, zoom_y+ZSIZE, img);
		}
		for (j=0; j<ZSIZE; j++)
		{
			xor_pixel(zoom_x-1, zoom_y+j, img);
			xor_pixel(zoom_x+ZSIZE, zoom_y+j, img);
		}
	}
#endif
}

int render_thumb(void *thumb, int size, int bzip2, pixel *vid_buf, int px, int py, int scl)
{
	unsigned char *d,*c=thumb;
	int i,j,x,y,a,t,r,g,b,sx,sy;

	if (bzip2)
	{
		if (size<16)
			return 1;
		if (c[3]!=0x74 || c[2]!=0x49 || c[1]!=0x68 || c[0]!=0x53)
			return 1;
		if (c[4]>PT_NUM)
			return 2;
		if (c[5]!=CELL || c[6]!=XRES/CELL || c[7]!=YRES/CELL)
			return 3;
		i = XRES*YRES;
		d = malloc(i);
		if (!d)
			return 1;

		if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+8), size-8, 0, 0))
			return 1;
		size = i;
	}
	else
		d = c;

	if (size < XRES*YRES)
	{
		if (bzip2)
			free(d);
		return 1;
	}

	sy = 0;
	for (y=0; y+scl<=YRES; y+=scl)
	{
		sx = 0;
		for (x=0; x+scl<=XRES; x+=scl)
		{
			a = 0;
			r = g = b = 0;
			for (j=0; j<scl; j++)
				for (i=0; i<scl; i++)
				{
					t = d[(y+j)*XRES+(x+i)];
					if (t==0xFF)
					{
						r += 256;
						g += 256;
						b += 256;
						a += 2;
					}
					else if (t)
					{
						if (t>=PT_NUM)
							goto corrupt;
						r += PIXR(ptypes[t].pcolors);
						g += PIXG(ptypes[t].pcolors);
						b += PIXB(ptypes[t].pcolors);
						a ++;
					}
				}
			if (a)
			{
				a = 256/a;
				r = (r*a)>>8;
				g = (g*a)>>8;
				b = (b*a)>>8;
			}

			drawpixel(vid_buf, px+sx, py+sy, r, g, b, 255);
			sx++;
		}
		sy++;
	}

	if (bzip2)
		free(d);
	return 0;

corrupt:
	if (bzip2)
		free(d);
	return 1;
}

//draws the cursor
void render_cursor(pixel *vid, int x, int y, int t, int rx, int ry)
{
#ifdef OGLR
	int i;
	if (t<PT_NUM||(t&0xFF)==PT_LIFE||t==SPC_AIR||t==SPC_HEAT||t==SPC_COOL||t==SPC_VACUUM||t==SPC_WIND||t==SPC_PGRV||t==SPC_NGRV||t==SPC_PROP)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_XOR);
		glBegin(GL_LINE_LOOP);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		y *= sdl_scale;
		x *= sdl_scale;
		ry *= sdl_scale;
		rx *= sdl_scale;
		if (CURRENT_BRUSH==SQUARE_BRUSH)
		{
			glVertex2f(x-rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)-ry+1);
			glVertex2f(x+rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)-ry+1);
			glVertex2f(x+rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)+ry+1);
			glVertex2f(x-rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)+ry+1);
			glVertex2f(x-rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)-ry+1);
		}
		else if (CURRENT_BRUSH==CIRCLE_BRUSH)
		{
			for (i = 0; i < 360; i++)
			{
			  float degInRad = i*(M_PI/180.0f);
			  glVertex2f((cos(degInRad)*rx)+x, (sin(degInRad)*ry)+/*(YRES+MENUSIZE)*sdl_scale-*/y);
			}
		}
		else if (CURRENT_BRUSH==TRI_BRUSH)
		{
			glVertex2f(x+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)+ry+1);
			glVertex2f(x+rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)-ry+1);
			glVertex2f(x-rx+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)-ry+1);
			glVertex2f(x+1, (/*(YRES+MENUSIZE)*sdl_scale-*/y)+ry+1);
		}
		glEnd();
		glDisable(GL_COLOR_LOGIC_OP);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
#else
	int i,j,c;
	if (t<PT_NUM||(t&0xFF)==PT_LIFE||t==SPC_AIR||t==SPC_HEAT||t==SPC_COOL||t==SPC_VACUUM||t==SPC_WIND||t==SPC_PGRV||t==SPC_NGRV||t==SPC_PROP)
	{
		if (rx<=0)
			for (j = y - ry; j <= y + ry; j++)
				xor_pixel(x, j, vid);
		else
		{
			int tempy = y, i, j, oldy;
			if (CURRENT_BRUSH == TRI_BRUSH)
				tempy = y + ry;
			for (i = x - rx; i <= x; i++) {
				oldy = tempy;
				if (!InCurrentBrush(i-x,tempy-y,rx,ry))
					continue;
				while (InCurrentBrush(i-x,tempy-y,rx,ry))
					tempy = tempy - 1;
				tempy = tempy + 1;
				if (oldy != tempy && CURRENT_BRUSH != SQUARE_BRUSH)
					oldy--;
				if (CURRENT_BRUSH == TRI_BRUSH)
					oldy = tempy;
				for (j = tempy; j <= oldy; j++) {
					int i2 = 2*x-i, j2 = 2*y-j;
					if (CURRENT_BRUSH == TRI_BRUSH)
						j2 = y+ry;
					xor_pixel(i, j, vid);
					if (i2 != i)
						xor_pixel(i2, j, vid);
					if (j2 != j)
						xor_pixel(i, j2, vid);
					if (i2 != i && j2 != j)
						xor_pixel(i2, j2, vid);
				}
			}
		}
	}
	else //wall cursor
	{
		int tc;
		c = (rx/CELL) * CELL;
		x = (x/CELL) * CELL;
		y = (y/CELL) * CELL;

		tc = !((c%(CELL*2))==0);

		x -= c/2;
		y -= c/2;

		x += tc*(CELL/2);
		y += tc*(CELL/2);

		for (i=0; i<CELL+c; i++)
		{
			xor_pixel(x+i, y, vid);
			xor_pixel(x+i, y+CELL+c-1, vid);
		}
		for (i=1; i<CELL+c-1; i++)
		{
			xor_pixel(x, y+i, vid);
			xor_pixel(x+CELL+c-1, y+i, vid);
		}
	}
#endif
}

SDL_VideoInfo info;
int sdl_opened = 0;
int sdl_open(void)
{
	char screen_err = 0;
#ifdef WIN32
	SDL_SysWMinfo SysInfo;
	HWND WindowHandle;
	HICON hIconSmall;
	HICON hIconBig;
#elif defined(LIN32) || defined(LIN64)
	SDL_Surface *icon;
#endif
	int status;
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
	
#ifdef WIN32
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWMInfo(&SysInfo) <= 0) {
	    printf("%s : %d\n", SDL_GetError(), SysInfo.window);
	    exit(-1);
	}
	WindowHandle = SysInfo.window;
	hIconSmall = (HICON)LoadImage(&__ImageBase, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	hIconBig = (HICON)LoadImage(&__ImageBase, MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#elif defined(LIN32) || defined(LIN64)
	icon = SDL_CreateRGBSurfaceFrom(app_icon, 16, 16, 32, 64, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_WM_SetIcon(icon, NULL);
#endif
	SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
	
	atexit(SDL_Quit);

	if(!sdl_opened)
		info = *SDL_GetVideoInfo(); 

	if (info.current_w<((XRES+BARSIZE)*sdl_scale) || info.current_h<((YRES+MENUSIZE)*sdl_scale))
	{
		sdl_scale = 1;
		screen_err = 1;
		fprintf(stderr, "Can't change scale factor, because screen resolution is too small");
	}
#if defined(OGLR)
	sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_OPENGL);
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

	if(sdl_opened)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0, (XRES+BARSIZE)*sdl_scale, 0, (YRES+MENUSIZE)*sdl_scale, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else
	{
#ifdef WIN32
		status = glewInit();
		if(status != GLEW_OK)
		{
			fprintf(stderr, "Initializing Glew: %d\n", status);
			return 0;
		}
#endif
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0, (XRES+BARSIZE)*sdl_scale, 0, (YRES+MENUSIZE)*sdl_scale, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glRasterPos2i(0, (YRES+MENUSIZE));
		glPixelZoom(1, -1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//FBO Texture
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &partsFboTex);
		glBindTexture(GL_TEXTURE_2D, partsFboTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, XRES, YRES, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		 
		//FBO
		glGenFramebuffers(1, &partsFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glEnable(GL_BLEND);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, partsFboTex, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Reset framebuffer binding
		glDisable(GL_TEXTURE_2D);

		//Texture for main UI
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &vidBuf);
		glBindTexture(GL_TEXTURE_2D, vidBuf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, XRES+BARSIZE, YRES+MENUSIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//Texture for air to be drawn
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &airBuf);
		glBindTexture(GL_TEXTURE_2D, airBuf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//Zoom texture
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &zoomTex);
		glBindTexture(GL_TEXTURE_2D, zoomTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//Texture for velocity maps for gravity
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &partsTFX);
		glBindTexture(GL_TEXTURE_2D, partsTFX);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &partsTFY);
		glBindTexture(GL_TEXTURE_2D, partsTFY);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//Texture for velocity maps for air
		//TODO: Combine all air maps into 3D array or structs
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &airVX);
		glBindTexture(GL_TEXTURE_2D, airVX);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &airVY);
		glBindTexture(GL_TEXTURE_2D, airVY);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &airPV);
		glBindTexture(GL_TEXTURE_2D, airPV);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//Fire alpha texture
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &fireAlpha);
		glBindTexture(GL_TEXTURE_2D, fireAlpha);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, CELL*3, CELL*3, 0, GL_ALPHA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//Glow alpha texture
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &glowAlpha);
		glBindTexture(GL_TEXTURE_2D, glowAlpha);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 11, 11, 0, GL_ALPHA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);


		//Blur Alpha texture
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &blurAlpha);
		glBindTexture(GL_TEXTURE_2D, blurAlpha);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 7, 7, 0, GL_ALPHA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		loadShaders();
	}
#else
#ifdef PIX16
	if (kiosk_enable)
		sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,16,SDL_FULLSCREEN|SDL_SWSURFACE);
	else
		sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,16,SDL_SWSURFACE);
#else
	if (kiosk_enable)
		sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_FULLSCREEN|SDL_SWSURFACE);
	else
		sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_SWSURFACE);
#endif
#endif
	if (!sdl_scrn)
	{
		fprintf(stderr, "Creating window: %s\n", SDL_GetError());
		return 0;
	}
	SDL_EnableUNICODE(1);
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#if (defined(LIN32) || defined(LIN64)) && defined(SDL_VIDEO_DRIVER_X11)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_VERSION(&sdl_wminfo.version);
	SDL_GetWMInfo(&sdl_wminfo);
	sdl_wminfo.info.x11.lock_func();
	XA_CLIPBOARD = XInternAtom(sdl_wminfo.info.x11.display, "CLIPBOARD", 1);
	XA_TARGETS = XInternAtom(sdl_wminfo.info.x11.display, "TARGETS", 1);
	sdl_wminfo.info.x11.unlock_func();
#endif

	if (screen_err)
		error_ui(vid_buf, 0, "Can't change scale factor, because screen resolution is too small");
	sdl_opened = 1;
	return 1;
}
#ifdef OGLR
void checkShader(GLuint shader, char * shname)
{
	GLuint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		char errorBuf[ GL_INFO_LOG_LENGTH];
		int errLen;
		glGetShaderInfoLog(shader, GL_INFO_LOG_LENGTH, &errLen, errorBuf);
		fprintf(stderr, "Failed to compile %s shader:\n%s\n", shname, errorBuf);
		exit(1);
	}
}
void checkProgram(GLuint program, char * progname)
{
	GLuint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		char errorBuf[ GL_INFO_LOG_LENGTH];
		int errLen;
		glGetShaderInfoLog(program, GL_INFO_LOG_LENGTH, &errLen, errorBuf);
		fprintf(stderr, "Failed to link %s program:\n%s\n", progname, errorBuf);
		exit(1);
	}
}
void loadShaders()
{
	GLuint vertexShader, fragmentShader;

	//Particle texture
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &fireVertex, NULL);
	glShaderSource( fragmentShader, 1, &fireFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "FV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "FF");

	fireProg = glCreateProgram();
	glAttachShader( fireProg, vertexShader );
	glAttachShader( fireProg, fragmentShader );
	glLinkProgram( fireProg );
	checkProgram(fireProg, "F");
	
	//Lensing
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &lensVertex, NULL);
	glShaderSource( fragmentShader, 1, &lensFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "LV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "LF");

	lensProg = glCreateProgram();
	glAttachShader( lensProg, vertexShader );
	glAttachShader( lensProg, fragmentShader );
	glLinkProgram( lensProg );
	checkProgram(lensProg, "L");
	
	//Air Velocity
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &airVVertex, NULL);
	glShaderSource( fragmentShader, 1, &airVFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "AVX");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "AVF");

	airProg_Velocity = glCreateProgram();
	glAttachShader( airProg_Velocity, vertexShader );
	glAttachShader( airProg_Velocity, fragmentShader );
	glLinkProgram( airProg_Velocity );
	checkProgram(airProg_Velocity, "AV");
	
	//Air Pressure
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &airPVertex, NULL);
	glShaderSource( fragmentShader, 1, &airPFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "APV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "APF");

	airProg_Pressure = glCreateProgram();
	glAttachShader( airProg_Pressure, vertexShader );
	glAttachShader( airProg_Pressure, fragmentShader );
	glLinkProgram( airProg_Pressure );
	checkProgram(airProg_Pressure, "AP");
	
	//Air cracker
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &airCVertex, NULL);
	glShaderSource( fragmentShader, 1, &airCFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "ACV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "ACF");

	airProg_Cracker = glCreateProgram();
	glAttachShader( airProg_Cracker, vertexShader );
	glAttachShader( airProg_Cracker, fragmentShader );
	glLinkProgram( airProg_Cracker );
	checkProgram(airProg_Cracker, "AC");
}
#endif
int draw_debug_info(pixel* vid, int lm, int lx, int ly, int cx, int cy, int line_x, int line_y)
{
	char infobuf[256];
	if(debug_flags & DEBUG_PERFORMANCE_FRAME || debug_flags & DEBUG_PERFORMANCE_CALC)
	{
		int t1, t2, x = 0, i = debug_perf_istart;
		float partiavg = 0, frameavg = 0;
		while(i != debug_perf_iend)
		{
			partiavg += abs(debug_perf_partitime[i]/100000);
			frameavg += abs(debug_perf_frametime[i]/100000);
			if(debug_flags & DEBUG_PERFORMANCE_CALC)
				t1 = abs(debug_perf_partitime[i]/100000);
			else
				t1 = 0;
				
			if(debug_flags & DEBUG_PERFORMANCE_FRAME)
				t2 = abs(debug_perf_frametime[i]/100000);
			else
				t2 = 0;
				
			if(t1 > YRES)
				t1 = YRES;
			if(t1+t2 > YRES)
				t2 = YRES-t1;
				
			if(t1>0)
				draw_line(vid, x, YRES, x, YRES-t1, 0, 255, 120, XRES+BARSIZE);
			if(t2>0)	
				draw_line(vid, x, YRES-t1, x, YRES-(t1+t2), 255, 120, 0, XRES+BARSIZE);
				
			i++;
			x++;
			i %= DEBUG_PERF_FRAMECOUNT;
		}
		
		if(debug_flags & DEBUG_PERFORMANCE_CALC)
			t1 = abs(partiavg / x);
		else
			t1 = 0;
			
		if(debug_flags & DEBUG_PERFORMANCE_FRAME)
			t2 = abs(frameavg / x);
		else
			t2 = 0;
		
		if(t1 > YRES)
			t1 = YRES;
		if(t1+t2 > YRES)
			t2 = YRES-t1;
		
		if(t1>0)
			fillrect(vid, x, YRES-t1-1, 5, t1+2, 0, 255, 0, 255);
		if(t2>0)	
			fillrect(vid, x, (YRES-t1)-t2-1, 5, t2+1, 255, 0, 0, 255);
	}
	if(debug_flags & DEBUG_DRAWTOOL)
	{
		if(lm == 1) //Line tool
		{
			blend_line(vid, 0, line_y, XRES, line_y, 255, 255, 255, 120);
			blend_line(vid, line_x, 0, line_x, YRES, 255, 255, 255, 120);
	
			blend_line(vid, 0, ly, XRES, ly, 255, 255, 255, 120);
			blend_line(vid, lx, 0, lx, YRES, 255, 255, 255, 120);
			
			sprintf(infobuf, "%d x %d", lx, ly);
			drawtext_outline(vid, lx+(lx>line_x?3:-textwidth(infobuf)-3), ly+(ly<line_y?-10:3), infobuf, 255, 255, 255, 200, 0, 0, 0, 120);
			
			sprintf(infobuf, "%d x %d", line_x, line_y);
			drawtext_outline(vid, line_x+(lx<line_x?3:-textwidth(infobuf)-2), line_y+(ly>line_y?-10:3), infobuf, 255, 255, 255, 200, 0, 0, 0, 120);
			
			sprintf(infobuf, "%d", abs(line_x-lx));
			drawtext_outline(vid, (line_x+lx)/2-textwidth(infobuf)/2, line_y+(ly>line_y?-10:3), infobuf, 255, 255, 255, 200, 0, 0, 0, 120);
			
			sprintf(infobuf, "%d", abs(line_y-ly));
			drawtext_outline(vid, line_x+(lx<line_x?3:-textwidth(infobuf)-2), (line_y+ly)/2-3, infobuf, 255, 255, 255, 200, 0, 0, 0, 120);
		}
	}
	if(debug_flags & DEBUG_PARTS)
	{
		int i = 0, x = 0, y = 0, lpx = 0, lpy = 0;
		sprintf(infobuf, "%d/%d (%.2f%%)", parts_lastActiveIndex, NPART, (((float)parts_lastActiveIndex)/((float)NPART))*100.0f);
		for(i = 0; i < NPART; i++){
			if(parts[i].type){
				drawpixel(vid, x, y, 255, 255, 255, 180);
			} else {
				drawpixel(vid, x, y, 0, 0, 0, 180);
			}
			if(i == parts_lastActiveIndex)
			{
				lpx = x;
				lpy = y;
			}
			x++;
			if(x>=XRES){
				y++;
				x = 0;
			}
		}
		draw_line(vid, 0, lpy, XRES, lpy, 0, 255, 120, XRES+BARSIZE);
		draw_line(vid, lpx, 0, lpx, YRES, 0, 255, 120, XRES+BARSIZE);
		drawpixel(vid, lpx, lpy, 255, 50, 50, 220);
				
		drawpixel(vid, lpx+1, lpy, 255, 50, 50, 120);
		drawpixel(vid, lpx-1, lpy, 255, 50, 50, 120);
		drawpixel(vid, lpx, lpy+1, 255, 50, 50, 120);
		drawpixel(vid, lpx, lpy-1, 255, 50, 50, 120);
		
		fillrect(vid, 7, YRES-26, textwidth(infobuf)+5, 14, 0, 0, 0, 180);		
		drawtext(vid, 10, YRES-22, infobuf, 255, 255, 255, 255);
	}
}
