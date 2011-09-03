#include <math.h>
#include <SDL/SDL.h>
#include <bzlib.h>

#ifdef OpenGL
#ifdef MACOSX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#include <defines.h>
#include <air.h>
#include <powder.h>
#include <graphics.h>
#define INCLUDE_FONTDATA
#include <font.h>
#include <misc.h>


unsigned cmode = CM_FIRE;
SDL_Surface *sdl_scrn;
int sdl_scale = 1;

unsigned char fire_r[YRES/CELL][XRES/CELL];
unsigned char fire_g[YRES/CELL][XRES/CELL];
unsigned char fire_b[YRES/CELL][XRES/CELL];

unsigned int fire_alpha[CELL*3][CELL*3];
pixel *fire_bg;
pixel *pers_bg;

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
		return NULL;
	}
	if(i != (width*height)*3){
		printf("Result buffer size mismatch, %d != %d\n", i, (width*height)*3);
		free(red_chan);
		free(green_chan);
		free(blue_chan);
		free(undata);
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
#ifdef OpenGL
	RenderScene();
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

//old and unused equation to draw walls
/*
void draw_tool(pixel *vid_buf, int b, int sl, int sr, unsigned pc, unsigned iswall)
{
	int x, y, i, j, c;
	int bo = b;
	if (iswall==1)
	{
		b = b-100;
		x = (2+32*((b-22)/1));
		y = YRES+2+40;
		switch (b)
		{
		case WL_WALLELEC:
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
		case 23:
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
		case 24:
			for (j=1; j<15; j+=2)
			{
				for (i=1+(1&(j>>1)); i<27; i+=2)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			break;
		case 25:
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
		case 26:
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
		case 27:
			for (j=1; j<15; j+=2)
			{
				for (i=1+(1&(j>>1)); i<27; i+=2)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			break;
		case 28:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					if (!(i%2) && !(j%2))
					{
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
					}
				}
			}
			break;
		case 29:
			for (j=1; j<15; j+=2)
			{
				for (i=1+(1&(j>>1)); i<27; i+=2)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			break;
		case 30:
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
		case 32:
			for (j=1; j<15; j+=2)
			{
				for (i=1+(1&(j>>1)); i<27; i+=2)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			break;
		case 33:
			for (j=1; j<15; j+=2)
			{
				for (i=1+(1&(j>>1)); i<27; i+=2)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			break;
		case 34:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					if (!(i%2) && !(j%2))
					{
						vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
					}
				}
			}
			break;
		case 36:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
			if (c<544)
			{
				c = 255;
			}
			else
			{
				c = 0;
			}
			drawtext(vid_buf, x+14-textwidth("AIR")/2, y+4, "AIR", c, c, c, 255);
			break;
		case 37:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
			if (c<544)
			{
				c = 255;
			}
			else
			{
				c = 0;
			}
			drawtext(vid_buf, x+14-textwidth("HEAT")/2, y+4, "HEAT", c, c, c, 255);
			break;
		case 38:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
			if (c<544)
			{
				c = 255;
			}
			else
			{
				c = 0;
			}
			drawtext(vid_buf, x+14-textwidth("COOL")/2, y+4, "COOL", c, c, c, 255);
			break;
		case 39:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
			c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
			if (c<544)
			{
				c = 255;
			}
			else
			{
				c = 0;
			}
			drawtext(vid_buf, x+14-textwidth("VAC")/2, y+4, "VAC", c, c, c, 255);
			break;
		default:
			for (j=1; j<15; j++)
			{
				for (i=1; i<27; i++)
				{
					vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
				}
			}
		}
		if (b==30)
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
		x = 2+32*(b/2);
		y = YRES+2+20*(b%2);
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
	if (bo==sl || bo==sr)
	{
		c = 0;
		if (bo==sl)
			c |= PIXPACK(0xFF0000);
		if (bo==sr)
			c |= PIXPACK(0x0000FF);
		for (i=0; i<30; i++)
		{
			vid_buf[(XRES+BARSIZE)*(y-1)+(x+i-1)] = c;
			vid_buf[(XRES+BARSIZE)*(y+16)+(x+i-1)] = c;
		}
		for (j=0; j<18; j++)
		{
			vid_buf[(XRES+BARSIZE)*(y+j-1)+(x-1)] = c;
			vid_buf[(XRES+BARSIZE)*(y+j-1)+(x+28)] = c;
		}
	}
}
*/
//draws walls and elements for menu
int draw_tool_xy(pixel *vid_buf, int x, int y, int b, unsigned pc)
{
	int i, j, c;
	pixel gc;
	if (x > XRES-26 || x < 0)
		return 26;
	if ((b&0xFF) == PT_LIFE)
	{
#ifdef OpenGL
		fillrect(vid_buf, x, y, 28, 16, PIXR(pc), PIXG(pc), PIXB(pc), 255);
#else
		for (j=1; j<15; j++)
		{
			for (i=1; i<27; i++)
			{
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
			}
		}
#endif
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
#ifdef OpenGL
		fillrect(vid_buf, x, y, 28, 16, PIXR(pc), PIXG(pc), PIXB(pc), 255);
#else
		for (j=1; j<15; j++)
		{
			for (i=1; i<27; i++)
			{
				vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
			}
		}
#endif
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

//draws a pixel, identical to blendpixel(), except blendpixel has OpenGL support
#if defined(WIN32) && !defined(__GNUC__)
_inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#else
inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#endif
{
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

int drawtext(pixel *vid, int x, int y, const char *s, int r, int g, int b, int a)
{
#ifdef OpenGL
#else
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
#endif
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
#ifdef OpenGL
#else
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
				case 'b':
					r = g = 0;
					b = 255;
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
				}
				x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
			}
		}
	}

	return rh;
	#endif
}

//draws a rectange, (x,y) are the top left coords.
void drawrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
#ifdef OpenGL
	glBegin(GL_LINE_LOOP);
	glColor4ub(r, g, b, a);
	glVertex2i(x, y);
	glVertex2i(x+w, y);
	glVertex2i(x+w, y+h);
	glVertex2i(x, y+h);
	glEnd();
#else
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
#endif
}

//draws a rectangle and fills it in as well.
void fillrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
#ifdef OpenGL
	glBegin(GL_QUADS);
	glColor4ub(r, g, b, a);
	glVertex2i(x, y);
	glVertex2i(x+w, y);
	glVertex2i(x+w, y+h);
	glVertex2i(x, y+h);
	glEnd();
#else
	int i,j;
	for (j=1; j<h; j++)
		for (i=1; i<w; i++)
			drawpixel(vid, x+i, y+j, r, g, b, a);
#endif // OpenGL
}

void clearrect(pixel *vid, int x, int y, int w, int h)
{
	int i;
	for (i=1; i<h; i++)
		memset(vid+(x+1+(XRES+BARSIZE)*(y+i)), 0, PIXELSIZE*(w-1));
}
//draws a line of dots, where h is the height. (why is this even here)
void drawdots(pixel *vid, int x, int y, int h, int r, int g, int b, int a)
{
#ifdef OpenGL
	int i;
	glBegin(GL_QUADS);
	glColor4ub(r, g, b, a);
	for (i = 0; i <= h; i +=2)
		glVertex2i(x, y+i);
	glEnd();
#else
	int i;
	for (i=0; i<=h; i+=2)
		drawpixel(vid, x, y+i, r, g, b, a);
#endif //OpenGL
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
#ifdef OpenGL
	if (x<0 || y<0 || x>=XRES || r>=YRES)
		return;
	if (a!=255)
	{
		glBegin (GL_QUADS);
		glColor4ub(r,g,b,a);
		glVertex2i(x, y);
	}
	vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
#else
	pixel t;
	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return;
	if (a!=255)
	{
		t = vid[y*(XRES+BARSIZE)+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
	}
	vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
#endif //OpenGL
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
	int x, y, i, j;
	pixel c;

	if (cmode == CM_PERS)//this should never happen anyway
		return;

	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			if (cmode == CM_PRESS)
			{
				if (pv[y][x] > 0.0f)
					c  = PIXRGB(clamp_flt(pv[y][x], 0.0f, 8.0f), 0, 0);//positive pressure is red!
				else
					c  = PIXRGB(0, 0, clamp_flt(-pv[y][x], 0.0f, 8.0f));//negative pressure is blue!
			}
			else if (cmode == CM_VEL)
			{
				c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),//vx adds red
					clamp_flt(pv[y][x], 0.0f, 8.0f),//pressure adds green
					clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));//vy adds blue
			}
			else if (cmode == CM_HEAT && aheat_enable)
			{
				float ttemp = hv[y][x]+(-MIN_TEMP);
				int caddress = restrict_flt((int)( restrict_flt(ttemp, 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
				c = PIXRGB((int)((unsigned char)color_data[caddress]*0.7f), (int)((unsigned char)color_data[caddress+1]*0.7f), (int)((unsigned char)color_data[caddress+2]*0.7f));
				//c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),//vx adds red
				//	clamp_flt(hv[y][x], 0.0f, 1600.0f),//heat adds green
				//	clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));//vy adds blue
			}
			else if (cmode == CM_CRACK)
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
}

void draw_grav_zones(pixel * vid)
{
	int x, y, i, j;
	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			if(gravmask[y][x])
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
	int x, y, i;
	float nx, ny, dist;

	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			if(fabsf(gravx[y][x]) <= 0.001f && fabsf(gravy[y][x]) <= 0.001f)
				continue;
			nx = x*CELL;
			ny = y*CELL;
			dist = fabsf(gravx[y][x])+fabsf(gravy[y][x]);
			for(i = 0; i < 4; i++)
			{
				nx -= gravx[y][x]*0.5f;
				ny -= gravy[y][x]*0.5f;
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
	if (x<0 || y<0 || x>=XRES || y>=YRES)
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
		xor_pixel(x+i, y+h-1, vid);
	}
	for (i=2; i<h; i+=2)
	{
		xor_pixel(x, y+i, vid);
		xor_pixel(x+w-1, y+i, vid);
	}
}

//the main function for drawing the particles
void draw_parts(pixel *vid)
{
	int i, x, y, t, nx, ny, r, s;
	int orbd[4] = {0, 0, 0, 0};
	int orbl[4] = {0, 0, 0, 0};
	int cr, cg, cb;
	float fr, fg, fb;
	float pt = R_TEMP;
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
	for (i = 0; i<=parts_lastActiveIndex; i++) {
#ifdef OpenGL
		if (cmode == CM_FANCY) //If fancy mode
		{

			if (t==PT_MWAX)
			{
				for (x=-1; x<=1; x++)
				{
					for (y=-1; y<=1; y++)
					{
						if ((abs(x) == 0) && (abs(y) == 0))
							blendpixel(vid,x+nx,y+ny,224,224,170,255);
						else if (abs(y) != 0 && abs(x) != 0)
							blendpixel(vid,x+nx,y+ny,224,224,170,20);
						else
							blendpixel(vid,x+nx,y+ny,224,224,170,40);
					}
				}

			}

			else if (t==PT_PLUT)
			{
				int tempx;
				int tempy;
				cr = 0x40;
				cg = 0x70;
				cb = 0x20;
				blendpixel(vid, nx, ny, cr, cg, cb, 192);
				blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
				blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
				blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
				blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
				for (tempx = 2; tempx < 10; tempx++) {
					for (tempy = 2; tempy < 10; tempy++) {
						blendpixel(vid, nx+tempx, ny-tempy, cr, cg, cb, 5);
						blendpixel(vid, nx-tempx, ny+tempy, cr, cg, cb, 5);
						blendpixel(vid, nx+tempx, ny+tempy, cr, cg, cb, 5);
						blendpixel(vid, nx-tempx, ny-tempy, cr, cg, cb, 5);
					}
				}
			}
		}
		if (parts[i].type) {
			//Do nothing
			t = parts[i].type;
			nx = (int)(parts[i].x+0.5f);
			ny = (int)(parts[i].y+0.5f);
			glBegin (GL_POINTS);
			glColor3ub (PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors));
			glVertex2d (nx, ny);
			glEnd ();
		}
#else
		if (parts[i].type) {
			t = parts[i].type;

			nx = (int)(parts[i].x+0.5f);
			ny = (int)(parts[i].y+0.5f);

			if(photons[ny][nx]&0xFF && !(ptypes[t].properties & TYPE_ENERGY))
				continue;

			if (t==PT_SOAP)
			{
				if ((parts[i].ctype&7) == 7)
					draw_line(vid, nx, ny, (int)(parts[parts[i].tmp].x+0.5f), (int)(parts[parts[i].tmp].y+0.5f), 245, 245, 220, XRES+BARSIZE);
			}
			
			if(t==PT_WIRE)
			{
			if (parts[i].ctype==0)
			    blendpixel(vid, nx, ny, 255, 204, 0, 255);
			else if(parts[i].ctype==1)
			    blendpixel(vid, nx, ny, 0, 0, 255, 255);
			else
			    blendpixel(vid, nx, ny, 255, 255, 255, 255);
			
			continue;
			}

			if (cmode!=CM_HEAT)
			{
				if (t==PT_STKM)
				{
					char buff[20];  //Buffer for HP
					pixel pc;

					if (mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3)) //If mous is in the head
					{
						sprintf(buff, "%3d", parts[i].life);  //Show HP
						drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
					}

					if ((int)player[2]<PT_NUM) pc = ptypes[(int)player[2]].pcolors;
					else pc = PIXPACK(0xFFFFFF);
					s = XRES+BARSIZE;
					//head
					draw_line(vid , nx-2, ny+2, nx+2, ny+2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					draw_line(vid , nx-2, ny-2, nx+2, ny-2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					draw_line(vid , nx-2, ny-2, nx-2, ny+2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					draw_line(vid , nx+2, ny-2, nx+2, ny+2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					//legs
					draw_line(vid , nx, ny+3, player[3], player[4], 255, 255, 255, s);
					draw_line(vid , player[3], player[4], player[7], player[8], 255, 255, 255, s);
					draw_line(vid , nx, ny+3, player[11], player[12], 255, 255, 255, s);
					draw_line(vid , player[11], player[12], player[15], player[16], 255, 255, 255, s);
				}
				else if (t==PT_STKM2)
				{
					char buff[20];  //Buffer for HP
					pixel pc;

					if (mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3)) //If mous is in the head
					{
						sprintf(buff, "%3d", parts[i].life);  //Show HP
						drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
					}

					if ((int)player2[2]<PT_NUM) pc = ptypes[(int)player2[2]].pcolors;
					else pc = PIXPACK(0xFFFFFF);
					s = XRES+BARSIZE;
					//head
					draw_line(vid , nx-2, ny+2, nx+2, ny+2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					draw_line(vid , nx-2, ny-2, nx+2, ny-2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					draw_line(vid , nx-2, ny-2, nx-2, ny+2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					draw_line(vid , nx+2, ny-2, nx+2, ny+2, PIXR(pc), PIXG(pc), PIXB(pc), s);
					//legs
					draw_line(vid , nx, ny+3, player2[3], player2[4], 100, 100, 255, s);
					draw_line(vid , player2[3], player2[4], player2[7], player2[8], 100, 100, 255, s);
					draw_line(vid , nx, ny+3, player2[11], player2[12], 100, 100, 255, s);
					draw_line(vid , player2[11], player2[12], player2[15], player2[16], 100, 100, 255, s);
				}
				if (cmode==CM_NOTHING && t!=PT_PIPE && t!=PT_SWCH && t!=PT_LCRY && t!=PT_PUMP && t!=PT_GPMP && t!=PT_PBCN && t!=PT_FILT && t!=PT_HSWC && t!=PT_PCLN && t!=PT_DEUT && t!=PT_WIFI && t!=PT_LIFE && t!=PT_PVOD)//nothing display but show needed color changes
				{
					if (t==PT_PHOT)
					{
						cg = 0;
						cb = 0;
						cr = 0;
						for (x=0; x<12; x++) {
							cr += (parts[i].ctype >> (x+18)) & 1;
							cb += (parts[i].ctype >>  x)     & 1;
						}
						for (x=0; x<14; x++)
							cg += (parts[i].ctype >> (x+9))  & 1;
						x = 624/(cr+cg+cb+1);
						cr *= x;
						cg *= x;
						cb *= x;
						cr = cr>255?255:cr;
						cg = cg>255?255:cg;
						cb = cb>255?255:cb;
						blendpixel(vid, nx, ny, cr, cg, cb, 255);
					}
					else
					{
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
						blendpixel(vid, nx, ny, cr, cg, cb, 255);
					}
				}
				else if (cmode==CM_GRAD)
				{
					float frequency = 0.05;
					int q = parts[i].temp-40;
					cr = sin(frequency*q) * 16 + PIXR(ptypes[t].pcolors);
					cg = sin(frequency*q) * 16 + PIXG(ptypes[t].pcolors);
					cb = sin(frequency*q) * 16 + PIXB(ptypes[t].pcolors);
					if (cr>=255)
						cr = 255;
					if (cg>=255)
						cg = 255;
					if (cb>=255)
						cb = 255;
					if (cr<=0)
						cr = 0;
					if (cg<=0)
						cg = 0;
					if (cb<=0)
						cb = 0;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (cmode==CM_FANCY && //all fancy mode effects go here, this is a list of exceptions to skip
				         t!=PT_FIRE && t!=PT_PLSM && t!=PT_WTRV &&
				         t!=PT_HFLM && t!=PT_SPRK && t!=PT_FIRW &&
				         t!=PT_DUST && t!=PT_FIRW && t!=PT_FWRK &&
				         t!=PT_NEUT && t!=PT_LAVA && t!=PT_BOMB &&
				         t!=PT_PHOT && t!=PT_THDR && t!=PT_SMKE &&
				         t!=PT_LCRY && t!=PT_SWCH && t!=PT_PCLN &&
				         t!=PT_PUMP && t!=PT_HSWC && t!=PT_FILT &&
				         t!=PT_GPMP && t!=PT_PBCN && t!=PT_LIFE &&
						 t!=PT_O2 && t!=PT_H2 && t!=PT_PVOD)
				{
					if (ptypes[parts[i].type].properties&TYPE_LIQUID) //special effects for liquids in fancy mode
					{
						if (parts[i].type==PT_DEUT)
						{
							cr = PIXR(ptypes[t].pcolors) + parts[i].life*1;
							cg = PIXG(ptypes[t].pcolors) + parts[i].life*2;
							cb = PIXB(ptypes[t].pcolors) + parts[i].life*4;
							if (cr>=255)
								cr = 255;
							if (cg>=255)
								cg = 255;
							if (cb>=255)
								cb = 255;
							blendpixel(vid, nx, ny, cr, cg, cb, 255);
							for (x=-1; x<=1; x++)
							{
								for (y=-1; y<=1; y++)
								{
									if ((abs(x) == 0) && (abs(y) == 0))
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,100);
									else if (abs(y) != 0 || abs(x) != 0)
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,50);
								}
							}
						}
						else if (parts[i].type==PT_LAVA && parts[i].life)
						{
							cr = parts[i].life * 2 + 0xE0;
							cg = parts[i].life * 1 + 0x50;
							cb = parts[i].life/2 + 0x10;
							if (cr>255) cr = 255;
							if (cg>192) cg = 192;
							if (cb>128) cb = 128;
							for (x=-1; x<=1; x++)
							{
								for (y=-1; y<=1; y++)
								{
									if ((abs(x) == 0) && (abs(y) == 0))
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,100);
									else if (abs(y) != 0 || abs(x) != 0)
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,50);
								}
							}
						}
						else if (parts[i].type==PT_GLOW)
						{
							fr = restrict_flt(parts[i].temp-(275.13f+32.0f), 0, 128)/50.0f;
							fg = restrict_flt(parts[i].ctype, 0, 128)/50.0f;
							fb = restrict_flt(parts[i].tmp, 0, 128)/50.0f;

							cr = restrict_flt(64.0f+parts[i].temp-(275.13f+32.0f), 0, 255);
							cg = restrict_flt(64.0f+parts[i].ctype, 0, 255);
							cb = restrict_flt(64.0f+parts[i].tmp, 0, 255);

							vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr, cg, cb);
							x = nx/CELL;
							y = ny/CELL;
							fg += fire_g[y][x];
							if (fg > 255) fg = 255;
							fire_g[y][x] = fg;
							fb += fire_b[y][x];
							if (fb > 255) fb = 255;
							fire_b[y][x] = fb;
							fr += fire_r[y][x];
							if (fr > 255) fr = 255;
							fire_r[y][x] = fr;

							for (x=-1; x<=1; x++)
							{
								for (y=-1; y<=1; y++)
								{
									if ((abs(x) == 0) && (abs(y) == 0))
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,100);
									else if (abs(y) != 0 || abs(x) != 0)
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,50);
								}
							}
						}
						else
						{
							cr = PIXR(ptypes[t].pcolors);
							cg = PIXG(ptypes[t].pcolors);
							cb = PIXB(ptypes[t].pcolors);
							for (x=-1; x<=1; x++)
							{
								for (y=-1; y<=1; y++)
								{
									if ((abs(x) == 0) && (abs(y) == 0))
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,100);
									else if (abs(y) != 0 || abs(x) != 0)
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,50);
								}
							}
						}
					}
					else if (ptypes[parts[i].type].properties&TYPE_GAS)
					{
						//if(parts[i].type!=PT_FIRE&&parts[i].type!=PT_SMKE&&parts[i].type!=PT_PLSM&&parts[i].type!=PT_WTRV)
						{
							cr = PIXR(ptypes[t].pcolors);
							cg = PIXG(ptypes[t].pcolors);
							cb = PIXB(ptypes[t].pcolors);
							for (x=-1; x<=1; x++)
							{
								for (y=-1; y<=1; y++)
								{
									if ((abs(x) == 0) && (abs(y) == 0))
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,180);
									else if (abs(y) != 0 && abs(x) != 0)
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,50);
									else
										blendpixel(vid,x+nx,y+ny,cr,cg,cb,80);
								}
							}
						}
					}
					else if (ptypes[parts[i].type].properties&PROP_RADIOACTIVE)
					{
						int tempx = 0;
						int tempy = 0;
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
						//blendpixel(vid, nx, ny, cr, cg, cb, 192);
						//blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						//blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						//blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						//blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						addpixel(vid, nx, ny, cr, cg, cb, 192);
						addpixel(vid, nx+1, ny, cr, cg, cb, 96);
						addpixel(vid, nx-1, ny, cr, cg, cb, 96);
						addpixel(vid, nx, ny+1, cr, cg, cb, 96);
						addpixel(vid, nx, ny-1, cr, cg, cb, 96);
						for (tempx = 2; tempx < 7; tempx++) {
							for (tempy = 2; tempy < 7; tempy++) {
								addpixel(vid, nx+tempx, ny-tempy, cr, cg, cb, 5);
								addpixel(vid, nx-tempx, ny+tempy, cr, cg, cb, 5);
								addpixel(vid, nx+tempx, ny+tempy, cr, cg, cb, 5);
								addpixel(vid, nx-tempx, ny-tempy, cr, cg, cb, 5);
								//blendpixel(vid, nx+tempx, ny-tempy, cr, cg, cb, 5);
								//blendpixel(vid, nx-tempx, ny+tempy, cr, cg, cb, 5);
								//blendpixel(vid, nx+tempx, ny+tempy, cr, cg, cb, 5);
								//blendpixel(vid, nx-tempx, ny-tempy, cr, cg, cb, 5);
							}
						}
					}
					else //if no fancy effects, draw a simple pixel
					{
						vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
					}
				}
				else if (cmode==CM_LIFE)
				{
					float frequency = 0.4;
					int q;
					if (!(parts[i].life<5))
						q = sqrt(parts[i].life);
					else
						q = parts[i].life;
					cr = sin(frequency*q) * 100 + 128;
					cg = sin(frequency*q) * 100 + 128;
					cb = sin(frequency*q) * 100 + 128;
					if (cr>=255)
						cr = 255;
					if (cg>=255)
						cg = 255;
					if (cb>=255)
						cb = 255;
					if (cr<=0)
						cr = 0;
					if (cg<=0)
						cg = 0;
					if (cb<=0)
						cb = 0;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (t==PT_QRTZ || t==PT_PQRT)
				{
					int z = parts[i].tmp - 5;//speckles!
					if (parts[i].temp>(ptransitions[t].thv-800.0f))//hotglow for quartz
					{
						float frequency = 3.1415/(2*ptransitions[t].thv-(ptransitions[t].thv-800.0f));
						int q = (parts[i].temp>ptransitions[t].thv)?ptransitions[t].thv-(ptransitions[t].thv-800.0f):parts[i].temp-(ptransitions[t].thv-800.0f);
						cr = sin(frequency*q) * 226 + (z * 16 + PIXR(ptypes[t].pcolors));
						cg = sin(frequency*q*4.55 +3.14) * 34 + (z * 16 + PIXR(ptypes[t].pcolors));
						cb = sin(frequency*q*2.22 +3.14) * 64 + (z * 16 + PIXR(ptypes[t].pcolors));
						if (cr>=255)
							cr = 255;
						if (cg>=255)
							cg = 255;
						if (cb>=255)
							cb = 255;
						if (cr<=0)
							cr = 0;
						if (cg<=0)
							cg = 0;
						if (cb<=0)
							cb = 0;
						blendpixel(vid, nx, ny, cr, cg, cb, 255);
					}
					else
					{
						cr = z * 16 + PIXR(ptypes[t].pcolors);
						cg = z * 16 + PIXG(ptypes[t].pcolors);
						cb = z * 16 + PIXB(ptypes[t].pcolors);
						cr = cr>255?255:cr;
						cg = cg>255?255:cg;
						cb = cb>255?255:cb;
						blendpixel(vid, nx, ny, cr, cg, cb, 255);
					}
				}
				else if (t==PT_CLST)
				{
					int z = parts[i].tmp - 5;//speckles!
					cr = z * 16 + PIXR(ptypes[t].pcolors);
					cg = z * 16 + PIXG(ptypes[t].pcolors);
					cb = z * 16 + PIXB(ptypes[t].pcolors);
					cr = cr>255?255:cr;
					cg = cg>255?255:cg;
					cb = cb>255?255:cb;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (t==PT_CBNW)
				{
					int z = parts[i].tmp2 - 20;//speckles!
					cr = z * 1 + PIXR(ptypes[t].pcolors);
					cg = z * 2 + PIXG(ptypes[t].pcolors);
					cb = z * 8 + PIXB(ptypes[t].pcolors);
					cr = cr>255?255:cr;
					cg = cg>255?255:cg;
					cb = cb>255?255:cb;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (t==PT_SPNG)
				{
					cr = PIXR(ptypes[t].pcolors) - parts[i].life*15;
					cg = PIXG(ptypes[t].pcolors) - parts[i].life*15;
					cb = PIXB(ptypes[t].pcolors) - parts[i].life*15;
					if (cr<=50)
						cr = 50;
					if (cg<=50)
						cg = 50;
					if (cb<=20)
						cb = 20;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);

				}
				else if (t==PT_LIFE && parts[i].ctype>=0 && parts[i].ctype < NGOLALT) {
					pixel pc;
					if (parts[i].ctype==NGT_LOTE)//colors for life states
					{
						if (parts[i].tmp==2)
							pc = PIXRGB(255, 128, 0);
						else if (parts[i].tmp==1)
							pc = PIXRGB(255, 255, 0);
						else
							pc = PIXRGB(255, 0, 0);
					}
					else if (parts[i].ctype==NGT_FRG2)//colors for life states
					{
						if (parts[i].tmp==2)
							pc = PIXRGB(0, 100, 50);
						else
							pc = PIXRGB(0, 255, 90);
					}
					else if (parts[i].ctype==NGT_STAR)//colors for life states
					{
						if (parts[i].tmp==4)
							pc = PIXRGB(0, 0, 128);
						else if (parts[i].tmp==3)
							pc = PIXRGB(0, 0, 150);
						else if (parts[i].tmp==2)
							pc = PIXRGB(0, 0, 190);
						else if (parts[i].tmp==1)
							pc = PIXRGB(0, 0, 230);
						else
							pc = PIXRGB(0, 0, 70);
					}
					else if (parts[i].ctype==NGT_FROG)//colors for life states
					{
						if (parts[i].tmp==2)
							pc = PIXRGB(0, 100, 0);
						else
							pc = PIXRGB(0, 255, 0);
					}
					else if (parts[i].ctype==NGT_BRAN)//colors for life states
					{
						if (parts[i].tmp==1)
							pc = PIXRGB(150, 150, 0);
						else
							pc = PIXRGB(255, 255, 0);
					} else {
						pc = gmenu[parts[i].ctype].colour;
					}
					blendpixel(vid, nx, ny, PIXR(pc), PIXG(pc), PIXB(pc), 255);
					if (cmode==CM_BLOB)
					{
						blendpixel(vid, nx+1, ny, PIXR(pc), PIXG(pc), PIXB(pc), 223);
						blendpixel(vid, nx-1, ny, PIXR(pc), PIXG(pc), PIXB(pc), 223);
						blendpixel(vid, nx, ny+1, PIXR(pc), PIXG(pc), PIXB(pc), 223);
						blendpixel(vid, nx, ny-1, PIXR(pc), PIXG(pc), PIXB(pc), 223);

						blendpixel(vid, nx+1, ny-1, PIXR(pc), PIXG(pc), PIXB(pc), 112);
						blendpixel(vid, nx-1, ny-1, PIXR(pc), PIXG(pc), PIXB(pc), 112);
						blendpixel(vid, nx+1, ny+1, PIXR(pc), PIXG(pc), PIXB(pc), 112);
						blendpixel(vid, nx-1, ny+1, PIXR(pc), PIXG(pc), PIXB(pc), 112);
					}
				}
				else if (t==PT_DEUT)
				{

					if (parts[i].life>=700&&(cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY))
					{
						x = nx/CELL;
						y = ny/CELL;
						cr = 20;
						cg = 20;
						cb = 20;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cr = PIXR(ptypes[t].pcolors) + parts[i].life*1;
						cg = PIXG(ptypes[t].pcolors) + parts[i].life*2;
						cb = PIXB(ptypes[t].pcolors) + parts[i].life*4;
						if (cr>=255)
							cr = 255;
						if (cg>=255)
							cg = 255;
						if (cb>=255)
							cb = 255;
						blendpixel(vid, nx, ny, cr, cg, cb, 255);

					}

				}
				else if (t==PT_DUST && parts[i].life >= 1)//dust colors!
				{
					x = nx;
					y = ny;
					cr = parts[i].flags;
					cg = parts[i].tmp;
					cb = parts[i].ctype;
					if (decorations_enable && parts[i].dcolour)
					{
						int a = (parts[i].dcolour>>24)&0xFF;
						cr = (a*((parts[i].dcolour>>16)&0xFF) + (255-a)*cr) >> 8;
						cg = (a*((parts[i].dcolour>>8)&0xFF) + (255-a)*cg) >> 8;
						cb = (a*((parts[i].dcolour)&0xFF) + (255-a)*cb) >> 8;
					}
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cg,cb,cr);//yes i know this pixel is different color than the glow... i don't know why
						cg = cg/4;
						cb = cb/4;
						cr = cr/4;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
						blendpixel(vid,x,y,cg,cb,cr,255);
				}
				else if (t==PT_GRAV)
				{
					cr = 20;
					cg = 20;
					cb = 20;
					if (parts[i].vx>0)
					{
						cr += (parts[i].vx)*GRAV_R;
						cg += (parts[i].vx)*GRAV_G;
						cb += (parts[i].vx)*GRAV_B;
					}
					if (parts[i].vy>0)
					{
						cr += (parts[i].vy)*GRAV_G;
						cg += (parts[i].vy)*GRAV_B;
						cb += (parts[i].vy)*GRAV_R;

					}
					if (parts[i].vx<0)
					{
						cr -= (parts[i].vx)*GRAV_B;
						cg -= (parts[i].vx)*GRAV_R;
						cb -= (parts[i].vx)*GRAV_G;

					}
					if (parts[i].vy<0)
					{
						cr -= (parts[i].vy)*GRAV_R2;
						cg -= (parts[i].vy)*GRAV_G2;
						cb -= (parts[i].vy)*GRAV_B2;
					}
					if (cr>255)
						cr=255;
					if (cr<0)
						cr=0;
					if (cg>255)
						cg=255;
					if (cg<0)
						cg=0;
					if (cb>255)
						cb=255;
					if (cb<0)
						cb=0;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (t==PT_WIFI)
				{
					float frequency = 0.0628;
					int q = parts[i].tmp;
					cr = sin(frequency*q + 0) * 127 + 128;
					cg = sin(frequency*q + 2) * 127 + 128;
					cb = sin(frequency*q + 4) * 127 + 128;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
					if (mousex==(nx) && mousey==(ny) && DEBUG_MODE)//draw lines connecting wifi channels
					{
						int z;
						for (z = 0; z<NPART; z++) {
							if (parts[z].type)
							{
								if (parts[z].type==PT_WIFI&&parts[z].tmp==parts[i].tmp)
									xor_line(nx,ny,(int)(parts[z].x+0.5f),(int)(parts[z].y+0.5f),vid);
							}
						}
					}
				}
				else if (t==PT_PRTI)
				{
					int nxo = 0;
					int nyo = 0;
					int fire_rv = 0;
					float drad = 0.0f;
					float ddist = 0.0f;
					orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
					for (r = 0; r < 4; r++) {
						ddist = ((float)orbd[r])/16.0f;
						drad = (M_PI * ((float)orbl[r]) / 180.0f)*1.41f;
						nxo = ddist*cos(drad);
						nyo = ddist*sin(drad);
						if (ny+nyo>0 && ny+nyo<YRES && nx+nxo>0 && nx+nxo<XRES) {
							addpixel(vid, nx+nxo, ny+nyo, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), 255-orbd[r]);
							if (cmode == CM_FIRE && r == 1) {
								fire_rv = fire_r[(ny+nyo)/CELL][(nx+nxo)/CELL];
								fire_rv += 1;
								if (fire_rv>255) fire_rv = 255;
								fire_r[(ny+nyo)/CELL][(nx+nxo)/CELL] = fire_rv;
							}
						}
						addpixel(vid, nx, ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), 200);
					}
					if (DEBUG_MODE) {//draw lines connecting portals
						blendpixel(vid,nx,ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors),255);
						if (mousex==(nx) && mousey==(ny))
						{
							int z;
							for (z = 0; z<NPART; z++) {
								if (parts[z].type)
								{
									if (parts[z].type==PT_PRTO&&parts[z].tmp==parts[i].tmp)
										xor_line(nx,ny,(int)(parts[z].x+0.5f),(int)(parts[z].y+0.5f),vid);
								}
							}
						}
					}
				}
				else if (t==PT_PRTO)
				{
					int nxo = 0;
					int nyo = 0;
					int fire_bv = 0;
					float drad = 0.0f;
					float ddist = 0.0f;
					orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
					for (r = 0; r < 4; r++) {
						ddist = ((float)orbd[r])/16.0f;
						drad = (M_PI * ((float)orbl[r]) / 180.0f)*1.41f;
						nxo = ddist*cos(drad);
						nyo = ddist*sin(drad);
						if (ny+nyo>0 && ny+nyo<YRES && nx+nxo>0 && nx+nxo<XRES) {
							addpixel(vid, nx+nxo, ny+nyo, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), 255-orbd[r]);
							if (cmode == CM_FIRE && r == 1) {
								fire_bv = fire_b[(ny+nyo)/CELL][(nx+nxo)/CELL];
								fire_bv += 1;
								if (fire_bv>255) fire_bv = 255;
								fire_b[(ny+nyo)/CELL][(nx+nxo)/CELL] = fire_bv;
							}
						}
						addpixel(vid, nx, ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), 200);
					}
					if (DEBUG_MODE) {//draw lines connecting portals
						blendpixel(vid,nx,ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors),255);
						if (mousex==(nx) && mousey==(ny))
						{
							int z;
							for (z = 0; z<NPART; z++) {
								if (parts[z].type)
								{
									if (parts[z].type==PT_PRTI&&parts[z].tmp==parts[i].tmp)
										xor_line(nx,ny,(int)(parts[z].x+0.5f),(int)(parts[z].y+0.5f),vid);
								}
							}
						}
					}
				}
				else if ((t==PT_BIZR||t==PT_BIZRG||t==PT_BIZRS)&&parts[i].ctype)
				{
					cg = 0;
					cb = 0;
					cr = 0;
					for (x=0; x<12; x++) {
						cr += (parts[i].ctype >> (x+18)) & 1;
						cb += (parts[i].ctype >>  x)     & 1;
					}
					for (x=0; x<14; x++)
						cg += (parts[i].ctype >> (x+9))  & 1;
					x = 624/(cr+cg+cb+1);
					cr *= x;
					cg *= x;
					cb *= x;
					cr = cr>255?255:cr;
					cg = cg>255?255:cg;
					cb = cb>255?255:cb;
					if (fabs(parts[i].vx)+fabs(parts[i].vy)>0 &&(cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)) {
						fg = 0;
						fb = 0;
						fr = 0;
						fg = cg/40 * fabs(parts[i].vx)+fabs(parts[i].vy);
						fb = cb/40 * fabs(parts[i].vx)+fabs(parts[i].vy);
						fr = cr/40 * fabs(parts[i].vx)+fabs(parts[i].vy);
						vid[ny*(XRES+BARSIZE)+nx] = PIXRGB((int)restrict_flt(cr, 0, 255), (int)restrict_flt(cg, 0, 255), (int)restrict_flt(cb, 0, 255));
						x = nx/CELL;
						y = ny/CELL;
						fg += fire_g[y][x];
						if (fg > 255) fg = 255;
						fire_g[y][x] = fg;
						fb += fire_b[y][x];
						if (fb > 255) fb = 255;
						fire_b[y][x] = fb;
						fr += fire_r[y][x];
						if (fr > 255) fr = 255;
						fire_r[y][x] = fr;
					}
					else
						blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (t==PT_PIPE)
				{
					if (parts[i].ctype==2)
					{
						cr = 50;
						cg = 1;
						cb = 1;
					}
					else if (parts[i].ctype==3)
					{
						cr = 1;
						cg = 50;
						cb = 1;
					}
					else if (parts[i].ctype==4)
					{
						cr = 1;
						cg = 1;
						cb = 50;
					}
					else if (parts[i].temp<272.15&&parts[i].ctype!=1)
					{
						if (parts[i].temp>173.25&&parts[i].temp<273.15)
						{
							cr = 50;
							cg = 1;
							cb = 1;
						}
						if (parts[i].temp>73.25&&parts[i].temp<=173.15)
						{
							cr = 1;
							cg = 50;
							cb = 1;
						}
						if (parts[i].temp>=0&&parts[i].temp<=73.15)
						{
							cr = 1;
							cg = 1;
							cb = 50;
						}
					}
					else
					{
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
					}
					if ((parts[i].tmp&0xFF)>0 && (parts[i].tmp&0xFF)<PT_NUM)
					{
						cr = PIXR(ptypes[parts[i].tmp&0xFF].pcolors);
						cg = PIXG(ptypes[parts[i].tmp&0xFF].pcolors);
						cb = PIXB(ptypes[parts[i].tmp&0xFF].pcolors);
					}
					blendpixel(vid, nx, ny, cr, cg, cb, 255);



				}
				else if (t==PT_INVIS && (pv[ny/CELL][nx/CELL]>4.0f ||pv[ny/CELL][nx/CELL]<-4.0f))
				{
					blendpixel(vid, nx, ny, 15, 0, 150, 100);
				}
				else if (t==PT_ACID)
				{
					if (parts[i].life>75) parts[i].life = 75;
					if (parts[i].life<49) parts[i].life = 49;
					s = (parts[i].life-49)*3;
					if (s==0) s = 1;
					cr = 0x86 + s*4;
					cg = 0x36 + s*1;
					cb = 0x90 + s*2;

					if (cr>=255)
						cr = 255;
					if (cg>=255)
						cg = 255;
					if (cb>=255)
						cb = 255;

					blendpixel(vid, nx, ny, cr, cg, cb, 255);

					if (cmode==CM_BLOB)
					{
						blendpixel(vid, nx+1, ny, cr, cg, cb, 223);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 223);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 223);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 223);

						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 112);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 112);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 112);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 112);
					}
				}
				else if (t==PT_NEUT)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
						cg = 8;
						cb = 12;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
					else
					{
						cr = 0x20;
						cg = 0xE0;
						cb = 0xFF;
						blendpixel(vid, nx, ny, cr, cg, cb, 192);
						blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}
				}
				else if (t==PT_FILT)
				{
					int temp_bin = (int)((parts[i].temp-273.0f)*0.025f);
					if (temp_bin < 0) temp_bin = 0;
					if (temp_bin > 25) temp_bin = 25;
					parts[i].ctype = 0x1F << temp_bin;
					cg = 0;
					cb = 0;
					cr = 0;
					for (x=0; x<12; x++) {
						cr += (parts[i].ctype >> (x+18)) & 1;
						cb += (parts[i].ctype >>  x)     & 1;
					}
					for (x=0; x<14; x++)
						cg += (parts[i].ctype >> (x+9))  & 1;
					x = 624/(cr+cg+cb+1);
					cr *= x;
					cg *= x;
					cb *= x;
					cr = cr>255?255:cr;
					cg = cg>255?255:cg;
					cb = cb>255?255:cb;
					blendpixel(vid, nx, ny, cr, cg, cb, 127);
				}
				else if (t==PT_BRAY && parts[i].tmp==0)
				{
					int trans = parts[i].life * 7;
					if (trans>255) trans = 255;
					if (parts[i].ctype) {
						cg = 0;
						cb = 0;
						cr = 0;
						for (x=0; x<12; x++) {
							cr += (parts[i].ctype >> (x+18)) & 1;
							cb += (parts[i].ctype >>  x)     & 1;
						}
						for (x=0; x<14; x++)
							cg += (parts[i].ctype >> (x+9))  & 1;
						x = 624/(cr+cg+cb+1);
						cr *= x;
						cg *= x;
						cb *= x;
						cr = cr>255?255:cr;
						cg = cg>255?255:cg;
						cb = cb>255?255:cb;
						blendpixel(vid, nx, ny, cr, cg, cb, trans);
					} else
						blendpixel(vid, nx, ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), trans);
				}
				else if (t==PT_BRAY && parts[i].tmp==1)
				{
					int trans = parts[i].life/4;
					if (trans>255) trans = 255;
					if (parts[i].ctype) {
						cg = 0;
						cb = 0;
						cr = 0;
						for (x=0; x<12; x++) {
							cr += (parts[i].ctype >> (x+18)) & 1;
							cb += (parts[i].ctype >>  x)     & 1;
						}
						for (x=0; x<14; x++)
							cg += (parts[i].ctype >> (x+9))  & 1;
						x = 624/(cr+cg+cb+1);
						cr *= x;
						cg *= x;
						cb *= x;
						cr = cr>255?255:cr;
						cg = cg>255?255:cg;
						cb = cb>255?255:cb;
						blendpixel(vid, nx, ny, cr, cg, cb, trans);
					} else
						blendpixel(vid, nx, ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), trans);
				}
				else if (t==PT_BRAY && parts[i].tmp==2)
				{
					int trans = parts[i].life*100;
					if (trans>255) trans = 255;
					blendpixel(vid, nx, ny, 255, 150, 50, trans);
				}
				else if (t==PT_PHOT)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						cg = 0;
						cb = 0;
						cr = 0;
						for (x=0; x<12; x++) {
							cr += (parts[i].ctype >> (x+18)) & 1;
							cb += (parts[i].ctype >>  x)     & 1;
						}
						for (x=0; x<14; x++)
							cg += (parts[i].ctype >> (x+9))  & 1;
						x = 624/(cr+cg+cb+1);
						cr *= x;
						cg *= x;
						cb *= x;
						vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr>255?255:cr,cg>255?255:cg,cb>255?255:cb);						
						cr >>= 4;
						cg >>= 4;
						cb >>= 4;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cg = 0;
						cb = 0;
						cr = 0;
						for (x=0; x<12; x++) {
							cr += (parts[i].ctype >> (x+18)) & 1;
							cb += (parts[i].ctype >>  x)     & 1;
						}
						for (x=0; x<14; x++)
							cg += (parts[i].ctype >> (x+9))  & 1;
						x = 624/(cr+cg+cb+1);
						cr *= x;
						cg *= x;
						cb *= x;
						cr = cr>255?255:cr;
						cg = cg>255?255:cg;
						cb = cb>255?255:cb;
						if(cmode == CM_PERS){
							if(parts[i].pavg[0] && parts[i].pavg[1])
							{
								draw_line(vid, nx, ny, parts[i].pavg[0], parts[i].pavg[1], cr, cg, cb, XRES+BARSIZE);
							}
							else
							{
								vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr, cg, cb);	
							}
						} else {
							blendpixel(vid, nx, ny, cr, cg, cb, 192);
							blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
							blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
							blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
							blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
							blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
							blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
							blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
							blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
						}
					}
				}
				//Life can be 11 too, so don't just check for 10
				else if (t==PT_SWCH && parts[i].life >= 10)
				{
					x = nx;
					y = ny;
					blendpixel(vid,x,y,17,217,24,255);
				}
				else if (t==PT_SMKE)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						x = nx/CELL;
						y = ny/CELL;
						cg = 10;
						cb = 10;
						cr = 10;
						cg += fire_g[y][x];
						if (cg > 50) cg = 50;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 50) cb = 50;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 50) cr = 50;
						fire_r[y][x] = cr;
					}
					else
					{
						for (x=-3; x<4; x++)
						{
							for (y=-3; y<4; y++)
							{
								if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
									blendpixel(vid,x+nx,y+ny,100,100,100,30);
								if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
									blendpixel(vid,x+nx,y+ny,100,100,100,10);
								if (abs(x)+abs(y) == 2)
									blendpixel(vid,x+nx,y+ny,100,100,100,20);
							}
						}
					}
				}
				else if (t==PT_WTRV)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						x = nx/CELL;
						y = ny/CELL;
						cg = PIXG(ptypes[t].pcolors)/3;
						cb = PIXB(ptypes[t].pcolors)/3;
						cr = PIXR(ptypes[t].pcolors)/3;
						cg += fire_g[y][x];
						if (cg > PIXG(ptypes[t].pcolors)/2) cg = PIXG(ptypes[t].pcolors)/2;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > PIXB(ptypes[t].pcolors)/2) cb = PIXB(ptypes[t].pcolors)/2;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > PIXR(ptypes[t].pcolors)/2) cr = PIXR(ptypes[t].pcolors)/2;
						fire_r[y][x] = cr;
					}
					else
					{
						for (x=-3; x<4; x++)
						{
							for (y=-3; y<4; y++)
							{
								if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
								if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
								if (abs(x)+abs(y) == 2)
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
							}
						}
					}
				}
				else if (t==PT_O2)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						x = nx/CELL;
						y = ny/CELL;
						cg = PIXG(ptypes[t].pcolors)/3;
						cb = PIXB(ptypes[t].pcolors)/3;
						cr = PIXR(ptypes[t].pcolors)/3;
						cg += fire_g[y][x];
						if (cg > PIXG(ptypes[t].pcolors)/2) cg = PIXG(ptypes[t].pcolors)/2;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > PIXB(ptypes[t].pcolors)/2) cb = PIXB(ptypes[t].pcolors)/2;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > PIXR(ptypes[t].pcolors)/2) cr = PIXR(ptypes[t].pcolors)/2;
						fire_r[y][x] = cr;
					}
					else
					{
						for (x=-3; x<4; x++)
						{
							for (y=-3; y<4; y++)
							{
								if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
								if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
								if (abs(x)+abs(y) == 2)
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
							}
						}
					}
				}
				else if (t==PT_H2)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						x = nx/CELL;
						y = ny/CELL;
						cg = PIXG(ptypes[t].pcolors)/3;
						cb = PIXB(ptypes[t].pcolors)/3;
						cr = PIXR(ptypes[t].pcolors)/3;
						cg += fire_g[y][x];
						if (cg > PIXG(ptypes[t].pcolors)/2) cg = PIXG(ptypes[t].pcolors)/2;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > PIXB(ptypes[t].pcolors)/2) cb = PIXB(ptypes[t].pcolors)/2;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > PIXR(ptypes[t].pcolors)/2) cr = PIXR(ptypes[t].pcolors)/2;
						fire_r[y][x] = cr;
					}
					else
					{
						for (x=-3; x<4; x++)
						{
							for (y=-3; y<4; y++)
							{
								if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
								if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
								if (abs(x)+abs(y) == 2)
									blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
							}
						}
					}
				}
				else if (t==PT_THDR)
				{
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
						cg = 16;
						cb = 20;
						cr = 12;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cr = 0xFF;
						cg = 0xFF;
						cb = 0xA0;
						blendpixel(vid, nx, ny, cr, cg, cb, 192);
						blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}
				}
				else if (t==PT_GLOW)
				{
					fr = restrict_flt(parts[i].temp-(275.13f+32.0f), 0, 128)/50.0f;
					fg = restrict_flt(parts[i].ctype, 0, 128)/50.0f;
					fb = restrict_flt(parts[i].tmp, 0, 128)/50.0f;

					cr = restrict_flt(64.0f+parts[i].temp-(275.13f+32.0f), 0, 255);
					cg = restrict_flt(64.0f+parts[i].ctype, 0, 255);
					cb = restrict_flt(64.0f+parts[i].tmp, 0, 255);

					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr, cg, cb);
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						x = nx/CELL;
						y = ny/CELL;
						fg += fire_g[y][x];
						if (fg > 255) fg = 255;
						fire_g[y][x] = fg;
						fb += fire_b[y][x];
						if (fb > 255) fb = 255;
						fire_b[y][x] = fb;
						fr += fire_r[y][x];
						if (fr > 255) fr = 255;
						fire_r[y][x] = fr;
					}
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, cr, cg, cb, 223);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 223);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 223);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 223);

						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 112);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 112);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 112);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 112);
					}
				}
				else if (t==PT_LCRY)
				{
					uint8 GR = 0x50+((parts[i].life>10?10:parts[i].life)*10);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, GR, GR);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, GR, GR, GR, 223);
						blendpixel(vid, nx-1, ny, GR, GR, GR, 223);
						blendpixel(vid, nx, ny+1, GR, GR, GR, 223);
						blendpixel(vid, nx, ny-1, GR, GR, GR, 223);

						blendpixel(vid, nx+1, ny-1, GR, GR, GR, 112);
						blendpixel(vid, nx-1, ny-1, GR, GR, GR, 112);
						blendpixel(vid, nx+1, ny+1, GR, GR, GR, 112);
						blendpixel(vid, nx-1, ny+1, GR, GR, GR, 112);
					}
				}
				else if (t==PT_PCLN)
				{
					uint8 GR = 0x3B+((parts[i].life>10?10:parts[i].life)*19);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, GR, 10);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, GR, GR, 10, 223);
						blendpixel(vid, nx-1, ny, GR, GR, 10, 223);
						blendpixel(vid, nx, ny+1, GR, GR, 10, 223);
						blendpixel(vid, nx, ny-1, GR, GR, 10, 223);

						blendpixel(vid, nx+1, ny-1, GR, GR, 10, 112);
						blendpixel(vid, nx-1, ny-1, GR, GR, 10, 112);
						blendpixel(vid, nx+1, ny+1, GR, GR, 10, 112);
						blendpixel(vid, nx-1, ny+1, GR, GR, 10, 112);
					}
				}
				else if (t==PT_PBCN)
				{
					uint8 GR = 0x3B+((parts[i].life>10?10:parts[i].life)*19);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, GR/2, 10);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, GR, GR/2, 10, 223);
						blendpixel(vid, nx-1, ny, GR, GR/2, 10, 223);
						blendpixel(vid, nx, ny+1, GR, GR/2, 10, 223);
						blendpixel(vid, nx, ny-1, GR, GR/2, 10, 223);

						blendpixel(vid, nx+1, ny-1, GR, GR/2, 10, 112);
						blendpixel(vid, nx-1, ny-1, GR, GR/2, 10, 112);
						blendpixel(vid, nx+1, ny+1, GR, GR/2, 10, 112);
						blendpixel(vid, nx-1, ny+1, GR, GR/2, 10, 112);
					}
				}
				else if (t==PT_DLAY)
				{
					int stage = (int)(((float)parts[i].life/(parts[i].temp-273.15))*100.0f);
					cr = PIXR(ptypes[t].pcolors)+stage;
					cg = PIXG(ptypes[t].pcolors)+stage;
					cb = PIXB(ptypes[t].pcolors)+stage;
					if(cr>255)
						cr = 255;
					if(cg>255)
						cg = 255;
					if(cb>255)
						cb = 255;
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr, cg, cb);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, cr, cg, cb, 223);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 223);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 223);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 223);

						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 112);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 112);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 112);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 112);
					}
				}
				else if (t==PT_HSWC)
				{
					uint8 GR = 0x3B+((parts[i].life>10?10:parts[i].life)*19);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, 10, 10);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, GR, 10, 10, 223);
						blendpixel(vid, nx-1, ny, GR, 10, 10, 223);
						blendpixel(vid, nx, ny+1, GR, 10, 10, 223);
						blendpixel(vid, nx, ny-1, GR, 10, 10, 223);

						blendpixel(vid, nx+1, ny-1, GR, 10, 10, 112);
						blendpixel(vid, nx-1, ny-1, GR, 10, 10, 112);
						blendpixel(vid, nx+1, ny+1, GR, 10, 10, 112);
						blendpixel(vid, nx-1, ny+1, GR, 10, 10, 112);
					}
				}
				else if (t==PT_PVOD)
				{
					uint8 GR = 0x3B+((parts[i].life>10?10:parts[i].life)*16);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, 30, 30);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, GR, 30, 30, 223);
						blendpixel(vid, nx-1, ny, GR, 30, 30, 223);
						blendpixel(vid, nx, ny+1, GR, 30, 30, 223);
						blendpixel(vid, nx, ny-1, GR, 30, 30, 223);

						blendpixel(vid, nx+1, ny-1, GR, 30, 30, 112);
						blendpixel(vid, nx-1, ny-1, GR, 30, 30, 112);
						blendpixel(vid, nx+1, ny+1, GR, 30, 30, 112);
						blendpixel(vid, nx-1, ny+1, GR, 30, 30, 112);
					}
				}
				else if (t==PT_PUMP)
				{
					uint8 GR = 0x3B+((parts[i].life>10?10:parts[i].life)*19);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(10, 10, GR);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, 10, 10, GR, 223);
						blendpixel(vid, nx-1, ny, 10, 10, GR, 223);
						blendpixel(vid, nx, ny+1, 10, 10, GR, 223);
						blendpixel(vid, nx, ny-1, 10, 10, GR, 223);

						blendpixel(vid, nx+1, ny-1, 10, 10, GR, 112);
						blendpixel(vid, nx-1, ny-1, 10, 10, GR, 112);
						blendpixel(vid, nx+1, ny+1, 10, 10, GR, 112);
						blendpixel(vid, nx-1, ny+1, 10, 10, GR, 112);
					}
				}
				else if (t==PT_GPMP)
				{
					uint8 GR = 0x3B+((parts[i].life>10?10:parts[i].life)*19);
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(10, GR, GR);
					if (cmode == CM_BLOB) {
						blendpixel(vid, nx+1, ny, 10, GR, GR, 223);
						blendpixel(vid, nx-1, ny, 10, GR, GR, 223);
						blendpixel(vid, nx, ny+1, 10, GR, GR, 223);
						blendpixel(vid, nx, ny-1, 10, GR, GR, 223);

						blendpixel(vid, nx+1, ny-1, 10, GR, GR, 112);
						blendpixel(vid, nx-1, ny-1, 10, GR, GR, 112);
						blendpixel(vid, nx+1, ny+1, 10, GR, GR, 112);
						blendpixel(vid, nx-1, ny+1, 10, GR, GR, 112);
					}
				}
				else if (t==PT_PLSM)
				{
					float ttemp = (float)parts[i].life;
					int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
					uint8 R = plasma_data[caddress];
					uint8 G = plasma_data[caddress+1];
					uint8 B = plasma_data[caddress+2];
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						cr = R/8;
						cg = G/8;
						cb = B/8;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cr = R;
						cg = G;
						cb = B;
						blendpixel(vid, nx, ny, cr, cg, cb, 192);
						blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}
				} else if (t==PT_HFLM)
				{
					float ttemp = (float)parts[i].life;
					int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
					uint8 R = hflm_data[caddress];
					uint8 G = hflm_data[caddress+1];
					uint8 B = hflm_data[caddress+2];
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						cr = R/8;
						cg = G/8;
						cb = B/8;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cr = R;
						cg = G;
						cb = B;
						blendpixel(vid, nx, ny, cr, cg, cb, 192);
						blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}
				} else if (t==PT_FIRW&&parts[i].tmp>=3)
				{
					float ttemp = (float)parts[i].tmp-4;
					int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
					uint8 R = firw_data[caddress];
					uint8 G = firw_data[caddress+1];
					uint8 B = firw_data[caddress+2];
					if (decorations_enable && parts[i].dcolour)
					{
						int a = (parts[i].dcolour>>24)&0xFF;
						R = (a*((parts[i].dcolour>>16)&0xFF) + (255-a)*R) >> 8;
						G = (a*((parts[i].dcolour>>8)&0xFF) + (255-a)*G) >> 8;
						B = (a*((parts[i].dcolour)&0xFF) + (255-a)*B) >> 8;
					}
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						cr = R/2;
						cg = G/2;
						cb = B/2;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cr = R;
						cg = G;
						cb = B;
						blendpixel(vid, nx, ny, cr, cg, cb, 192);
						blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}
				}
				else if (t==PT_BOMB)
				{
					if (parts[i].tmp==0) {
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
						if (cmode != CM_CRACK) {
							int newx = 0;
							float flicker = rand()%20;
							float gradv = flicker + fabs(parts[i].vx)*17 + fabs(parts[i].vy)*17;
							blendpixel(vid, nx, ny, cr, cg, cb, (gradv*4)>255?255:(gradv*4) );
							blendpixel(vid, nx+1, ny, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							blendpixel(vid, nx-1, ny, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							blendpixel(vid, nx, ny+1, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							blendpixel(vid, nx, ny-1, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							if (gradv>255) gradv=255;
							blendpixel(vid, nx+1, ny-1, cr, cg, cb, gradv);
							blendpixel(vid, nx-1, ny-1, cr, cg, cb, gradv);
							blendpixel(vid, nx+1, ny+1, cr, cg, cb, gradv);
							blendpixel(vid, nx-1, ny+1, cr, cg, cb, gradv);
							for (newx = 1; gradv>0.5; newx++) {
								addpixel(vid, nx+newx, ny, cr, cg, cb, gradv);
								addpixel(vid, nx-newx, ny, cr, cg, cb, gradv);

								addpixel(vid, nx, ny+newx, cr, cg, cb, gradv);
								addpixel(vid, nx, ny-newx, cr, cg, cb, gradv);
								gradv = gradv/1.2f;
							}
						} else {
							blendpixel(vid, nx, ny, cr, cg, cb, 255);
						}
					}
					else if (parts[i].tmp==1) {
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
						if (cmode != CM_CRACK) {
							int newx = 0;
							float flicker = rand()%20;
							float gradv = 4*parts[i].life + flicker;
							for (newx = 0; gradv>0.5; newx++) {
								addpixel(vid, nx+newx, ny, cr, cg, cb, gradv);
								addpixel(vid, nx-newx, ny, cr, cg, cb, gradv);

								addpixel(vid, nx, ny+newx, cr, cg, cb, gradv);
								addpixel(vid, nx, ny-newx, cr, cg, cb, gradv);
								gradv = gradv/1.5f;
							}
						} else {
							blendpixel(vid, nx, ny, cr, cg, cb, 255);
						}
					}
					else {
						blendpixel(vid, nx, ny, 255, 255, 255, 255);
					}

				}
				else if (t==PT_GBMB)
				{
					if (parts[i].life<=0) {//not yet detonated
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
						if (cmode != CM_CRACK) {
							int newx = 0;
							float flicker = rand()%20;
							float gradv = flicker + fabs(parts[i].vx)*17 + fabs(parts[i].vy)*17;
							blendpixel(vid, nx, ny, cr, cg, cb, (gradv*4)>255?255:(gradv*4) );
							blendpixel(vid, nx+1, ny, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							blendpixel(vid, nx-1, ny, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							blendpixel(vid, nx, ny+1, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							blendpixel(vid, nx, ny-1, cr, cg, cb, (gradv*2)>255?255:(gradv*2) );
							if (gradv>255) gradv=255;
							blendpixel(vid, nx+1, ny-1, cr, cg, cb, gradv);
							blendpixel(vid, nx-1, ny-1, cr, cg, cb, gradv);
							blendpixel(vid, nx+1, ny+1, cr, cg, cb, gradv);
							blendpixel(vid, nx-1, ny+1, cr, cg, cb, gradv);
							for (newx = 1; gradv>0.5; newx++) {
								addpixel(vid, nx+newx, ny, cr, cg, cb, gradv);
								addpixel(vid, nx-newx, ny, cr, cg, cb, gradv);

								addpixel(vid, nx, ny+newx, cr, cg, cb, gradv);
								addpixel(vid, nx, ny-newx, cr, cg, cb, gradv);
								gradv = gradv/1.2f;
							}
						} else {
							blendpixel(vid, nx, ny, cr, cg, cb, 255);
						}
					}
					else {//exploding
						cr = PIXR(ptypes[t].pcolors);
						cg = PIXG(ptypes[t].pcolors);
						cb = PIXB(ptypes[t].pcolors);
						if (cmode != CM_CRACK) {
							int newx = 0;
							float flicker = rand()%20;
							float gradv = 4*parts[i].life + flicker;
							for (newx = 0; gradv>0.5; newx++) {
								addpixel(vid, nx+newx, ny, cr, cg, cb, gradv);
								addpixel(vid, nx-newx, ny, cr, cg, cb, gradv);

								addpixel(vid, nx, ny+newx, cr, cg, cb, gradv);
								addpixel(vid, nx, ny-newx, cr, cg, cb, gradv);
								gradv = gradv/1.5f;
							}
						} else {
							blendpixel(vid, nx, ny, cr, cg, cb, 255);
						}
					}
				}
				else if (ptypes[t].properties&PROP_HOT_GLOW && parts[i].temp>(ptransitions[t].thv-800.0f))
				{
					float frequency = 3.1415/(2*ptransitions[t].thv-(ptransitions[t].thv-800.0f));
					int q = (parts[i].temp>ptransitions[t].thv)?ptransitions[t].thv-(ptransitions[t].thv-800.0f):parts[i].temp-(ptransitions[t].thv-800.0f);
					cr = sin(frequency*q) * 226 + PIXR(ptypes[t].pcolors);
					cg = sin(frequency*q*4.55 +3.14) * 34 + PIXG(ptypes[t].pcolors);
					cb = sin(frequency*q*2.22 +3.14) * 64 + PIXB(ptypes[t].pcolors);
					if (cr>=255)
						cr = 255;
					if (cg>=255)
						cg = 255;
					if (cb>=255)
						cb = 255;
					if (cr<=0)
						cr = 0;
					if (cg<=0)
						cg = 0;
					if (cb<=0)
						cb = 0;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else if (t==PT_FIRE && parts[i].life)
				{
					float ttemp = (float)((int)(parts[i].life/2));
					int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
					uint8 R = flm_data[caddress];
					uint8 G = flm_data[caddress+1];
					uint8 B = flm_data[caddress+2];
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						cr = R/8;
						cg = G/8;
						cb = B/8;
						x = nx/CELL;
						y = ny/CELL;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
					}
					else
					{
						cr = parts[i].life * 8;
						cg = parts[i].life * 2;
						cb = parts[i].life;
						if (cr>255) cr = 255;
						if (cg>192) cg = 212;
						if (cb>128) cb = 192;
						blendpixel(vid, nx, ny, cr, cg, cb, 255);
						blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
						blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
						blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
						blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
						blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}
					// Older Code
					/*if(cmode == 3||cmode==4 || cmode==6)
					{
					    cr = parts[i].life / 4;
					    cg = parts[i].life / 16;
					    cb = parts[i].life / 32;
					    if(cr>255) cr = 255;
					    if(cg>192) cg = 212;
					    if(cb>128) cb = 192;
					    x = nx/CELL;
					    y = ny/CELL;
					    cr += fire_r[y][x];
					    if(cr > 255) cr = 255;
					    fire_r[y][x] = cr;
					    cg += fire_g[y][x];
					    if(cg > 255) cg = 255;
					    fire_g[y][x] = cg;
					    cb += fire_b[y][x];
					    if(cb > 255) cb = 255;
					    fire_b[y][x] = cb;
					}
					else
					{
					    cr = parts[i].life * 8;
					    cg = parts[i].life * 2;
					    cb = parts[i].life;
					    if(cr>255) cr = 255;
					    if(cg>192) cg = 212;
					    if(cb>128) cb = 192;
					    blendpixel(vid, nx, ny, cr, cg, cb, 255);
					    blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
					    blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
					    blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
					    blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
					    blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
					    blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
					    blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
					    blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
					}*/
				}
				else if (t==PT_LAVA && parts[i].life)
				{
					cr = parts[i].life * 2 + 0xE0;
					cg = parts[i].life * 1 + 0x50;
					cb = parts[i].life/2 + 0x10;
					if (cr>255) cr = 255;
					if (cg>192) cg = 192;
					if (cb>128) cb = 128;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
					blendpixel(vid, nx+1, ny, cr, cg, cb, 64);
					blendpixel(vid, nx-1, ny, cr, cg, cb, 64);
					blendpixel(vid, nx, ny+1, cr, cg, cb, 64);
					blendpixel(vid, nx, ny-1, cr, cg, cb, 64);
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						cr /= 32;
						cg /= 32;
						cb /= 32;
						x = nx/CELL;
						y = ny/CELL;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				else if (t==PT_LAVA || t==PT_SPRK)
				{
					vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
					if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY)
					{
						if (t == PT_LAVA)
						{
							cr = 3;
							cg = i%2;
							cb = 0;
						}
						else
						{
							cr = 8;
							cg = 12;
							cb = 16;
						}
						x = nx/CELL;
						y = ny/CELL;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				/*else if((t==PT_COAL || t==PT_BCOL) && parts[i].tmp2 > 100.0f-80.0f){
					float frequency = 3.1415/(2*100.0f-(100.0f-80.0f));
					int q = (parts[i].tmp2>100.0f)?100.0f-(100.0f-80.0f):parts[i].tmp2-(100.0f-80.0f);
					cr = PIXR(ptypes[t].pcolors);
					cg = PIXG(ptypes[t].pcolors);
					cb = PIXB(ptypes[t].pcolors);
					
					cr += parts[i].tmp2;
					cg += parts[i].tmp2;
					cb += parts[i].tmp2;
					
					
					cr += sin(frequency*q) * 226;
					cg += sin(frequency*q*4.55 +3.14) * 34;
					cb += sin(frequency*q*2.22 +3.14) * 64;
					
					if (cr>=255)
						cr = 255;
					if (cg>=255)
						cg = 255;
					if (cb>=255)
						cb = 255;
					if (cr<=0)
						cr = 0;
					if (cg<=0)
						cg = 0;
					if (cb<=0)
						cb = 0;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}*/
				else if(t==PT_COAL || t==PT_BCOL){
					cr = PIXR(ptypes[t].pcolors);
					cg = PIXG(ptypes[t].pcolors);
					//cb = PIXB(ptypes[t].pcolors);
					
					cr += (parts[i].tmp2-295.15f)/3;
					//cg += (parts[i].tmp2-273.15f)/3;
					//cb += (parts[i].tmp2-273.15f)/3;
					if (cr>=170)
						cr = 170;
					if (cr<=cg)
						cr = cg;
						
					cg = cb = cr;
					
					if((parts[i].temp-295.15f) > 300.0f-200.0f)
					{
						float frequency = 3.1415/(2*300.0f-(300.0f-200.0f));
						int q = ((parts[i].temp-295.15f)>300.0f)?300.0f-(300.0f-200.0f):(parts[i].temp-295.15f)-(300.0f-200.0f);
					
						cr += sin(frequency*q) * 226;
						cg += sin(frequency*q*4.55 +3.14) * 34;
						cb += sin(frequency*q*2.22 +3.14) * 64;
					}
					
					if (cr>=255)
						cr = 255;
					if (cg>=255)
						cg = 255;
					if (cb>=255)
						cb = 255;
					if (cr<=0)
						cr = 0;
					if (cg<=0)
						cg = 0;
					if (cb<=0)
						cb = 0;
					blendpixel(vid, nx, ny, cr, cg, cb, 255);
				}
				else //if no special effect, draw a simple pixel
					vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
			}
			else //heat view
			{
				float ttemp = parts[i].temp+(-MIN_TEMP);
				int caddress = restrict_flt((int)( restrict_flt(ttemp, 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
				uint8 R = color_data[caddress];
				uint8 G = color_data[caddress+1];
				uint8 B = color_data[caddress+2];

				if (t==PT_STKM) //Stick man should be visible in heat mode
				{
					char buff[10];  //Buffer for HP

					if (mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3)) //If mous is in the head
					{
						sprintf(buff, "%3d", parts[i].life);  //Show HP
						drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
					}

					for (r=-2; r<=1; r++)
					{
						s = XRES+BARSIZE;
						vid[(ny-2)*s+nx+r] = PIXRGB (R, G, B);
						vid[(ny+2)*s+nx+r+1] =  PIXRGB (R, G, B);
						vid[(ny+r+1)*s+nx-2] =  PIXRGB (R, G, B);
						vid[(ny+r)*s+nx+2] =  PIXRGB (R, G, B);
					}
					draw_line(vid , nx, ny+3, player[3], player[4], R, G, B, s);
					draw_line(vid , player[3], player[4], player[7], player[8], R, G, B, s);
					draw_line(vid , nx, ny+3, player[11], player[12], R, G, B, s);
					draw_line(vid , player[11], player[12], player[15], player[16], R, G, B, s);
				}
				else if (t==PT_STKM2) //Stick man should be visible in heat mode
				{
					char buff[10];  //Buffer for HP

					if (mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3)) //If mous is in the head
					{
						sprintf(buff, "%3d", parts[i].life);  //Show HP
						drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
					}

					for (r=-2; r<=1; r++)
					{
						s = XRES+BARSIZE;
						vid[(ny-2)*s+nx+r] = PIXRGB (R, G, B);
						vid[(ny+2)*s+nx+r+1] =  PIXRGB (R, G, B);
						vid[(ny+r+1)*s+nx-2] =  PIXRGB (R, G, B);
						vid[(ny+r)*s+nx+2] =  PIXRGB (R, G, B);
					}
					draw_line(vid , nx, ny+3, player2[3], player2[4], R, G, B, s);
					draw_line(vid , player2[3], player2[4], player2[7], player2[8], R, G, B, s);
					draw_line(vid , nx, ny+3, player2[11], player2[12], R, G, B, s);
					draw_line(vid , player2[11], player2[12], player2[15], player2[16], R, G, B, s);
				}
				else
				{
					vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(R, G, B);
					//blendpixel(vid, nx+1, ny, R, G, B, 255);
				}
			}
			//blob view!
			if (cmode == CM_BLOB&&t!=PT_FIRE&&t!=PT_PLSM&&t!=PT_HFLM&&t!=PT_NONE&&t!=PT_ACID&&t!=PT_LCRY&&t!=PT_GLOW&&t!=PT_SWCH&&t!=PT_SMKE&&t!=PT_WTRV&&!(t==PT_FIRW&&parts[i].tmp==3)&&t!=PT_LIFE&&t!=PT_H2&&t!=PT_O2)
			{
				if (t==PT_PHOT) {
					cg = 0;
					cb = 0;
					cr = 0;
					for (x=0; x<12; x++) {
						cr += (parts[i].ctype >> (x+18)) & 1;
						cb += (parts[i].ctype >>  x)     & 1;
					}
					for (x=0; x<14; x++)
						cg += (parts[i].ctype >> (x+9))  & 1;
					x = 624/(cr+cg+cb+1);
					cr *= x;
					cg *= x;
					cb *= x;
					cr = cr>255?255:cr;
					cg = cg>255?255:cg;
					cb = cb>255?255:cb;
				} else {
					cr = PIXR(ptypes[t].pcolors);
					cg = PIXG(ptypes[t].pcolors);
					cb = PIXB(ptypes[t].pcolors);
				}

				//if(vid[(ny-1)*YRES+(nx-1)]!=0){
				//	blendpixel(vid, nx, ny-1, R, G, B, 46);
				//}

				if (decorations_enable && parts[i].dcolour)
				{
					int a = (parts[i].dcolour>>24)&0xFF;
					cr = (a*((parts[i].dcolour>>16)&0xFF) + (255-a)*cr) >> 8;
					cg = (a*((parts[i].dcolour>>8)&0xFF) + (255-a)*cg) >> 8;
					cb = (a*((parts[i].dcolour)&0xFF) + (255-a)*cb) >> 8;
				}
				blendpixel(vid, nx+1, ny, cr, cg, cb, 223);
				blendpixel(vid, nx-1, ny, cr, cg, cb, 223);
				blendpixel(vid, nx, ny+1, cr, cg, cb, 223);
				blendpixel(vid, nx, ny-1, cr, cg, cb, 223);

				blendpixel(vid, nx+1, ny-1, cr, cg, cb, 112);
				blendpixel(vid, nx-1, ny-1, cr, cg, cb, 112);
				blendpixel(vid, nx+1, ny+1, cr, cg, cb, 112);
				blendpixel(vid, nx-1, ny+1, cr, cg, cb, 112);
			}
			if (decorations_enable && cmode!=CM_HEAT && cmode!=CM_LIFE && parts[i].dcolour)
				if(t==PT_LCRY){
					cr = (parts[i].dcolour>>16)&0xFF;
					cg = (parts[i].dcolour>>8)&0xFF;
					cb = (parts[i].dcolour)&0xFF;
					
					if(parts[i].life<10){
						cr /= 10-parts[i].life;
						cg /= 10-parts[i].life;
						cb /= 10-parts[i].life;
					}
					
					/*cr = cr>255?255:cr;
					cg = cg>255?255:cg;
					cb = cb>255?255:cb;*/
					blendpixel(vid, nx, ny, cr, cg, cb, (parts[i].dcolour>>24)&0xFF);
				} else {
					blendpixel(vid, nx, ny, (parts[i].dcolour>>16)&0xFF, (parts[i].dcolour>>8)&0xFF, (parts[i].dcolour)&0xFF, (parts[i].dcolour>>24)&0xFF);
				}
		}
#endif
	}
#ifdef OpenGL
	glFlush ();
#endif
}
void draw_walls(pixel *vid)
{
	int x, y, i, j, cr, cg, cb;
	unsigned char wt;
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
				
				if (cmode==CM_BLOB)
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
}

void create_decorations(int x, int y, int rx, int ry, int r, int g, int b, int click)
{
	int i,j,rp;
	if (rx==0 && ry==0)
	{
		rp = pmap[y][x];
		if (!rp)
			return;
		if (click == 4)
			parts[rp>>8].dcolour = 0;
		else
			parts[rp>>8].dcolour = ((255<<24)|(r<<16)|(g<<8)|b);
		return;
	}
	for (j=-ry; j<=ry; j++)
		for (i=-rx; i<=rx; i++)
			if(y+j>=0 && x+i>=0 && x+i<XRES && y+j<YRES)
				if (InCurrentBrush(i, j, rx, ry)){
					rp = pmap[y+j][x+i];
					if (!rp)
						continue;
					if (click == 4)
						parts[rp>>8].dcolour = 0;
					else
						parts[rp>>8].dcolour = ((255<<24)|(r<<16)|(g<<8)|b);
				}
}
void line_decorations(int x1, int y1, int x2, int y2, int rx, int ry, int r, int g, int b, int click)
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
			create_decorations(y, x, rx, ry, r, g, b, click);
		else
			create_decorations(x, y, rx, ry, r, g, b, click);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry))
			{
				if (cp)
					create_decorations(y, x, rx, ry, r, g, b, click);
				else
					create_decorations(x, y, rx, ry, r, g, b, click);
			}
			e -= 1.0f;
		}
	}
}
void box_decorations(int x1, int y1, int x2, int y2, int r, int g, int b, int click)
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
			create_decorations(i, j, 0, 0, r, g, b, click);
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
			for (j=0; j<14; j++)
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
				sprintf(buff, "Pressure: %3.2f", pv[signs[i].y/CELL][signs[i].x/CELL]);  //...pressure
				drawtext(vid_buf, x+3, y+3, buff, 255, 255, 255, 255);
			}
			if (strcmp(signs[i].text, "{t}")==0)
			{
				if (pmap[signs[i].y][signs[i].x])
					sprintf(buff, "Temp: %4.2f", parts[pmap[signs[i].y][signs[i].x]>>8].temp-273.15);  //...tempirature
				else
					sprintf(buff, "Temp: 0.00");  //...tempirature
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
				b = SDL_GetMouseState(&mx, &my);
				mx /= sdl_scale;
				my /= sdl_scale;
				signs[i].x = mx;
				signs[i].y = my;
			}
		}
}

void render_gravlensing(pixel *src, pixel * dst)
{
	int nx, ny, rx, ry, gx, gy, bx, by;
	int r, g, b;
	pixel t;
	for(nx = 0; nx < XRES; nx++)
	{
		for(ny = 0; ny < YRES; ny++)
		{
			rx = (int)(nx-gravxf[(ny*XRES)+nx]*0.75f+0.5f);
			ry = (int)(ny-gravyf[(ny*XRES)+nx]*0.75f+0.5f);
			gx = (int)(nx-gravxf[(ny*XRES)+nx]*0.875f+0.5f);
			gy = (int)(ny-gravyf[(ny*XRES)+nx]*0.875f+0.5f);
			bx = (int)(nx-gravxf[(ny*XRES)+nx]+0.5f);
			by = (int)(ny-gravyf[(ny*XRES)+nx]+0.5f);
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
				for (y=-CELL+1; y<2*CELL; y++)
					for (x=-CELL+1; x<2*CELL; x++)
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
	int x,y,i,j;
	float multiplier = 255.0f*intensity;
	float temp[CELL*3][CELL*3];
	memset(temp, 0, sizeof(temp));
	for (x=0; x<CELL; x++)
		for (y=0; y<CELL; y++)
			for (i=-CELL; i<CELL; i++)
				for (j=-CELL; j<CELL; j++)
					temp[y+CELL+j][x+CELL+i] += expf(-0.1f*(i*i+j*j));
	for (x=0; x<CELL*3; x++)
		for (y=0; y<CELL*3; y++)
			fire_alpha[y][x] = (int)(multiplier*temp[y][x]/(CELL*CELL));
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
}

//gets the thumbnail preview for stamps
pixel *prerender_save(void *save, int size, int *width, int *height)
{
	unsigned char *d,*c=save;
	int i,j,k,x,y,rx,ry,p=0;
	int bw,bh,w,h,new_format = 0;
	pixel *fb;

	if (size<16)
		return NULL;
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		return NULL;
	if (c[2]==0x43 && c[1]==0x75 && c[0]==0x66) {
		new_format = 1;
	}
	if (c[4]>SAVE_VERSION)
		return NULL;

	bw = c[6];
	bh = c[7];
	w = bw*CELL;
	h = bh*CELL;

	if (c[5]!=CELL)
		return NULL;

	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	d = malloc(i);
	if (!d)
		return NULL;
	fb = calloc(w*h, PIXELSIZE);
	if (!fb)
	{
		free(d);
		return NULL;
	}

	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), size-12, 0, 0))
		goto corrupt;
	size = i;

	if (size < bw*bh)
		goto corrupt;

	k = 0;
	for (y=0; y<bh; y++)
		for (x=0; x<bw; x++)
		{
			rx = x*CELL;
			ry = y*CELL;
			switch (d[p])
			{
			case 1:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case 2:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case 3:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
				break;
			case 4:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x8080FF);
				k++;
				break;
			case 6:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF8080);
				break;
			case 7:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(i&j&1))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case 8:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
						else
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_WALL:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_DESTROYALL:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_ALLOWLIQUID:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
				break;
			case WL_FAN:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x8080FF);
				k++;
				break;
			case WL_DETECT:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF8080);
				break;
			case WL_EWALL:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(i&j&1))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_WALLELEC:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
						else
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			}
			p++;
		}
	p += 2*k;
	if (p>=size)
		goto corrupt;

	for (y=0; y<h; y++)
		for (x=0; x<w; x++)
		{
			if (p >= size)
				goto corrupt;
			j=d[p++];
			if (j<PT_NUM && j>0)
			{
				if (j==PT_STKM || j==PT_STKM2)
				{
					pixel lc, hc=PIXRGB(255, 224, 178);
					if (j==PT_STKM) lc = PIXRGB(255, 255, 255);
					else lc = PIXRGB(100, 100, 255);
					//only need to check upper bound of y coord - lower bounds and x<w are checked in draw_line
					draw_line(fb , x-2, y-2, x+2, y-2, PIXR(hc), PIXG(hc), PIXB(hc), w);
					if (y+2<h)
					{
						draw_line(fb , x-2, y+2, x+2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
						draw_line(fb , x-2, y-2, x-2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
						draw_line(fb , x+2, y-2, x+2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
					}
					if (y+6<h)
					{
						draw_line(fb , x, y+3, x-1, y+6, PIXR(lc), PIXG(lc), PIXB(lc), w);
						draw_line(fb , x, y+3, x+1, y+6, PIXR(lc), PIXG(lc), PIXB(lc), w);
					}
					if (y+12<h)
					{
						draw_line(fb , x-1, y+6, x-3, y+12, PIXR(lc), PIXG(lc), PIXB(lc), w);
						draw_line(fb , x+1, y+6, x+3, y+12, PIXR(lc), PIXG(lc), PIXB(lc), w);
					}
				}
				else
					fb[y*w+x] = ptypes[j].pcolors;
			}
		}

	free(d);
	*width = w;
	*height = h;
	return fb;

corrupt:
	free(d);
	free(fb);
	return NULL;
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
	int i,j,c;
	if (t<PT_NUM||(t&0xFF)==PT_LIFE||t==SPC_AIR||t==SPC_HEAT||t==SPC_COOL||t==SPC_VACUUM||t==SPC_WIND||t==SPC_PGRV||t==SPC_NGRV)
	{
		if (rx<=0)
			xor_pixel(x, y, vid);
		else if (ry<=0)
			xor_pixel(x, y, vid);
		if (rx+ry<=0)
			xor_pixel(x, y, vid);
		else if (CURRENT_BRUSH==SQUARE_BRUSH)
		{
			for (j=0; j<=ry; j++)
				for (i=0; i<=rx; i++)
					if (i*j<=ry*rx && ((i+1)>rx || (j+1)>ry))
					{
						xor_pixel(x+i, y+j, vid);
						xor_pixel(x-i, y-j, vid);
						if (i&&j)xor_pixel(x+i, y-j, vid);
						if (i&&j)xor_pixel(x-i, y+j, vid);
					}
		}
		else if (CURRENT_BRUSH==CIRCLE_BRUSH)
		{
			for (j=0; j<=ry; j++)
				for (i=0; i<=rx; i++)
					if (pow(i,2)*pow(ry,2)+pow(j,2)*pow(rx,2)<=pow(rx,2)*pow(ry,2) &&
					  (pow(i+1,2)*pow(ry,2)+pow(j,2)*pow(rx,2)>pow(rx,2)*pow(ry,2) ||
					   pow(i,2)*pow(ry,2)+pow(j+1,2)*pow(rx,2)>pow(rx,2)*pow(ry,2)))
					{
						xor_pixel(x+i, y+j, vid);
						if (j) xor_pixel(x+i, y-j, vid);
						if (i) xor_pixel(x-i, y+j, vid);
						if (i&&j) xor_pixel(x-i, y-j, vid);
					}
		}
		else if (CURRENT_BRUSH==TRI_BRUSH)
 		{
			for (j=-ry; j<=ry; j++)
				for (i=-rx; i<=0; i++)
					if ((j <= ry ) && ( j >= (((-2.0*ry)/(rx))*i)-ry ) && (j+1>ry || ( j-1 < (((-2.0*ry)/(rx))*i)-ry )) )
						{
							xor_pixel(x+i, y+j, vid);
							if (i) xor_pixel(x-i, y+j, vid);
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
}

int sdl_open(void)
{
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
	atexit(SDL_Quit);
#ifdef OpenGL
	sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_OPENGL);
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	Enable2D ();
	//   glBegin(GL_TRIANGLES);
	//   glColor3ub (255, 0, 0);
	//   glVertex2d (0, 0);
	//   glColor3ub (0, 255, 0);
	//   glVertex2d (640,0);
	//    glColor3ub (0, 0, 255);
	//    glVertex2d (50, 50);
	//glEnd ();
	//glFlush ();
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
	SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
	sdl_seticon();
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
	return 1;
}

int draw_debug_info(pixel* vid, int lm, int lx, int ly, int cx, int cy, int line_x, int line_y)
{
	char infobuf[256];
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

#ifdef OpenGL
void Enable2D ()
{
	int ViewPort[4]; //Holds screen info

	glGetIntegerv (GL_VIEWPORT, ViewPort);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();

	glOrtho (0, ViewPort[2], ViewPort[3], 0, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST);
}
void RenderScene ()
{
	SDL_GL_SwapBuffers ();
}
void ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT);
}
#endif
