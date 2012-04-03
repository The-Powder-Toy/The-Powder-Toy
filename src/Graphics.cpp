#include <math.h>
#include <SDL/SDL.h>
#include <bzlib.h>
#include <string>

#if defined(OGLR)
#ifdef MACOSX
#include <GL/glew.h>
#include <OpenGL/gl.h>
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

#include "Config.h"
//#include "simulation/Air.h"
//#include "simulation/Gravity.h"
//#include "powder.h"
//#define INCLUDE_PSTRUCT
//#include "Simulation.h"
//#include "Graphics.h"
//#include "ElementGraphics.h"
#define INCLUDE_FONTDATA
#include "font.h"
#include "Misc.h"
#include "hmap.h"

//unsigned cmode = CM_FIRE;
unsigned int *render_modes;
unsigned int render_mode;
unsigned int colour_mode;
unsigned int *display_modes;
unsigned int display_mode;

//SDL_Surface *sdl_scrn;
int sdl_scale = 1;

#ifdef OGLR
GLuint zoomTex, vidBuf, airBuf, fireAlpha, glowAlpha, blurAlpha, partsFboTex, partsFbo, partsTFX, partsTFY, airPV, airVY, airVX;
GLuint fireProg, airProg_Pressure, airProg_Velocity, airProg_Cracker, lensProg;
#endif

/*
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
float plasma_data_pos[] = {1.0f, 0.9f, 0.5f, 0.25, 0.0f};*/

char * Graphics::GenerateGradient(pixel * colours, float * points, int pointcount, int size)
{
	int cp, i, j;
	pixel ptemp;
	char * newdata = (char*)malloc(size * 3);
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

void *Graphics::ptif_pack(pixel *src, int w, int h, int *result_size){
	int i = 0, datalen = (w*h)*3, cx = 0, cy = 0;
	unsigned char *red_chan = (unsigned char*)calloc(1, w*h);
	unsigned char *green_chan = (unsigned char*)calloc(1, w*h);
	unsigned char *blue_chan = (unsigned char*)calloc(1, w*h);
	unsigned char *data = (unsigned char*)malloc(((w*h)*3)+8);
	unsigned char *result = (unsigned char*)malloc(((w*h)*3)+8);
	
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
	
	if(BZ2_bzBuffToBuffCompress((char *)(result+8), (unsigned *)&i, (char *)data, datalen, 9, 0, 0) != 0){
		free(data);
		free(result);
		return NULL;
	}
	
	*result_size = i+8;
	free(data);
	return result;
}

pixel *Graphics::ptif_unpack(void *datain, int size, int *w, int *h){
	int width, height, i, cx, cy, resCode;
	unsigned char *red_chan;
	unsigned char *green_chan;
	unsigned char *blue_chan;
	unsigned char *data = (unsigned char*)datain;
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
	undata = (unsigned char*)calloc(1, (width*height)*3);
	red_chan = (unsigned char*)calloc(1, width*height);
	green_chan = (unsigned char*)calloc(1, width*height);
	blue_chan = (unsigned char *)calloc(1, width*height);
	result = (pixel *)calloc(width*height, PIXELSIZE);
	
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

pixel *Graphics::resample_img_nn(pixel * src, int sw, int sh, int rw, int rh)
{
	int y, x;
	pixel *q = NULL;
	q = (pixel *)malloc(rw*rh*PIXELSIZE);
	for (y=0; y<rh; y++)
		for (x=0; x<rw; x++){
			q[rw*y+x] = src[sw*(y*sh/rh)+(x*sw/rw)];
		}
	return q;
}

pixel *Graphics::resample_img(pixel *src, int sw, int sh, int rw, int rh)
{
	int y, x, fxceil, fyceil;
	//int i,j,x,y,w,h,r,g,b,c;
	pixel *q = NULL;
	//TODO: Actual resampling, this is just cheap nearest pixel crap
	if(rw == sw && rh == sh){
		//Don't resample
		q = (pixel *)malloc(rw*rh*PIXELSIZE);
		memcpy(q, src, rw*rh*PIXELSIZE);
	} else if(rw > sw && rh > sh){
		float fx, fy, fyc, fxc;
		double intp;
		pixel tr, tl, br, bl;
		q = (pixel *)malloc(rw*rh*PIXELSIZE);
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
		oq = (pixel *)malloc(sw*sh*PIXELSIZE);
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
			q = (pixel *)malloc(rw*rh*PIXELSIZE);
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

pixel *Graphics::rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f)
{
	int i,j,x,y,w,h,r,g,b,c;
	pixel p, *q;
	w = (sw+f-1)/f;
	h = (sh+f-1)/f;
	q = (pixel *)malloc(w*h*PIXELSIZE);
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
    glFlush();
    SDL_GL_SwapBuffers ();
}
#endif

//an easy way to draw a blob
void Graphics::drawblob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
	blendpixel(x+1, y, cr, cg, cb, 112);
	blendpixel(x-1, y, cr, cg, cb, 112);
	blendpixel(x, y+1, cr, cg, cb, 112);
	blendpixel(x, y-1, cr, cg, cb, 112);

	blendpixel(x+1, y-1, cr, cg, cb, 64);
	blendpixel(x-1, y-1, cr, cg, cb, 64);
	blendpixel(x+1, y+1, cr, cg, cb, 64);
	blendpixel(x-1, y+1, cr, cg, cb, 64);
}

//draws walls and elements for menu
/*int draw_tool_xy(pixel *vid_buf, int x, int y, int b, unsigned pc)
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
}*/

/*void draw_menu(pixel *vid_buf, int i, int hover)
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
}*/

/*void draw_color_menu(pixel *vid_buf, int i, int hover)
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
}*/

//draws a pixel, identical to blendpixel(), except blendpixel has OpenGL support
TPT_INLINE void Graphics::drawpixel(int x, int y, int r, int g, int b, int a)
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

inline int Graphics::drawchar(int x, int y, int c, int r, int g, int b, int a)
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
			drawpixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

inline int Graphics::addchar(int x, int y, int c, int r, int g, int b, int a)
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
			addpixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			}
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

int Graphics::drawtext(int x, int y, std::string &s, int r, int g, int b, int a)
{
	return drawtext(x, y, s.c_str(), r, g, b, a);
}

int Graphics::drawtext(int x, int y, const char *s, int r, int g, int b, int a)
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
			x = drawchar(x, y, *(unsigned char *)s, r, g, b, a);
	}
	return x;
}

//Draw text with an outline
int Graphics::drawtext_outline(int x, int y, const char *s, int r, int g, int b, int a, int olr, int olg, int olb, int ola)
{
	drawtext(x-1, y-1, s, olr, olg, olb, ola);
	drawtext(x+1, y+1, s, olr, olg, olb, ola);
	
	drawtext(x-1, y+1, s, olr, olg, olb, ola);
	drawtext(x+1, y-1, s, olr, olg, olb, ola);
	
	return drawtext(x, y, s, r, g, b, a);
}
int Graphics::drawtextwrap(int x, int y, int w, const char *s, int r, int g, int b, int a)
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
				}
				x = drawchar(x, y, *(unsigned char *)s, r, g, b, a);
			}
		}
	}

	return rh;
}

//draws a rectange, (x,y) are the top left coords.
void Graphics::drawrect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i;
	for (i=0; i<=w; i++)
	{
		drawpixel(x+i, y, r, g, b, a);
		drawpixel(x+i, y+h, r, g, b, a);
	}
	for (i=1; i<h; i++)
	{
		drawpixel(x, y+i, r, g, b, a);
		drawpixel(x+w, y+i, r, g, b, a);
	}
}

//draws a rectangle and fills it in as well.
void Graphics::fillrect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int i,j;
	for (j=1; j<h; j++)
		for (i=1; i<w; i++)
			drawpixel(x+i, y+j, r, g, b, a);
}

void Graphics::clearrect(int x, int y, int w, int h)
{
#ifdef OGLR
	fillrect(x, y, w, h, 0, 0, 0, 255);
#else
	int i;
	for (i=1; i<h; i++)
		memset(vid+(x+1+(XRES+BARSIZE)*(y+i)), 0, PIXELSIZE*(w-1));
#endif
}
//draws a line of dots, where h is the height. (why is this even here)
void Graphics::drawdots(int x, int y, int h, int r, int g, int b, int a)
{
	int i;
	for (i=0; i<=h; i+=2)
		drawpixel(x, y+i, r, g, b, a);
}

int Graphics::textwidth(char *s)
{
	int x = 0;
	for (; *s; s++)
		x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
	return x-1;
}

int Graphics::drawtextmax(int x, int y, int w, char *s, int r, int g, int b, int a)
{
	int i;
	w += x-5;
	for (; *s; s++)
	{
		if (x+font_data[font_ptrs[(int)(*(unsigned char *)s)]]>=w && x+textwidth(s)>=w+5)
			break;
		x = drawchar(x, y, *(unsigned char *)s, r, g, b, a);
	}
	if (*s)
		for (i=0; i<3; i++)
			x = drawchar(x, y, '.', r, g, b, a);
	return x;
}

int Graphics::textnwidth(char *s, int n)
{
	int x = 0;
	for (; *s; s++)
	{
		if (!n)
			break;
		if(((char)*s)=='\b')
		{
			s++;
			continue;
		}
		x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
		n--;
	}
	return x-1;
}
void Graphics::textnpos(char *s, int n, int w, int *cx, int *cy)
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

int Graphics::textwidthx(char *s, int w)
{
	int x=0,n=0,cw;
	for (; *s; s++)
	{
		if((char)*s == '\b')
		{
			s++;
			continue;
		}
		cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
		if (x+(cw/2) >= w)
			break;
		x += cw;
		n++;
	}
	return n;
}
int Graphics::textposxy(char *s, int width, int w, int h)
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
int Graphics::textwrapheight(char *s, int width)
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
TPT_INLINE void Graphics::blendpixel(int x, int y, int r, int g, int b, int a)
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

void Graphics::draw_icon(int x, int y, Icon icon)
{
	switch(icon)
	{
	case IconOpen:
		drawchar(x, y, 0x81, 255, 255, 255, 255);
		break;
	case IconReload:
		drawchar(x, y, 0x91, 255, 255, 255, 255);
		break;
	case IconSave:
		drawchar(x, y, 0x82, 255, 255, 255, 255);
		break;
	case IconVoteUp:
		drawchar(x, y, 0xCB, 0, 187, 18, 255);
		break;
	case IconVoteDown:
		drawchar(x, y, 0xCA, 187, 40, 0, 255);
		break;
	case IconTag:
		drawchar(x, y, 0x83, 255, 255, 255, 255);
		break;
	case IconNew:
		drawchar(x, y, 0x92, 255, 255, 255, 255);
		break;
	case IconLogin:
		drawchar(x, y, 0x84, 255, 255, 255, 255);
		break;
	case IconSimulationSettings:
		drawchar(x, y, 0xCF, 255, 255, 255, 255);
		break;
	case IconRenderSettings:
		addchar(x, y, 0xD8, 255, 0, 0, 255);
		addchar(x, y, 0xD9, 0, 255, 0, 255);
		addchar(x, y, 0xDA, 0, 0, 255, 255);
		break;
	case IconPause:
		drawchar(x, y, 0x90, 255, 255, 255, 255);
		break;
	case IconVoteSort:
	case IconDateSort:
	case IconFavourite:
	case IconFolder:
	case IconSearch:
	case IconDelete:
	default:
		drawchar(x, y, 't', 255, 255, 255, 255);
		break;
	}
}

void Graphics::draw_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a)  //Draws a line
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
void Graphics::addpixel(int x, int y, int r, int g, int b, int a)
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
void Graphics::xor_pixel(int x, int y)
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
void Graphics::xor_line(int x1, int y1, int x2, int y2)
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
			xor_pixel(y, x);
		else
			xor_pixel(x, y);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

//same as blend_pixel, but draws a line of it
void Graphics::blend_line(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
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
			blendpixel(y, x, r, g, b, a);
		else
			blendpixel(x, y, r, g, b, a);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

//same as xor_pixel, but draws a rectangle
void Graphics::xor_rect(int x, int y, int w, int h)
{
	int i;
	for (i=0; i<w; i+=2)
	{
		xor_pixel(x+i, y);
		xor_pixel(x+i, y+h-1);
	}
	for (i=2; i<h; i+=2)
	{
		xor_pixel(x, y+i);
		xor_pixel(x+w-1, y+i);
	}
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
#endif

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


//draws the photon colors in the HUD
void Graphics::draw_wavelengths(int x, int y, int h, int wl)
{
	int i,cr,cg,cb,j;
	int tmp;
	fillrect(x-1,y-1,30+1,h+1,64,64,64,255); // coords -1 size +1 to work around bug in fillrect - TODO: fix fillrect
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
			for (j=0; j<h; j++) blendpixel(x+29-i,y+j,cr>255?255:cr,cg>255?255:cg,cb>255?255:cb,255);
		}
	}
}

pixel *Graphics::render_packed_rgb(void *image, int width, int height, int cmp_size)
{
	unsigned char *tmp;
	pixel *res;
	int i;

	tmp = (unsigned char *)malloc(width*height*3);
	if (!tmp)
		return NULL;
	res = (pixel *)malloc(width*height*PIXELSIZE);
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

void Graphics::draw_rgba_image(unsigned char *data, int x, int y, float alpha)
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
			drawpixel(x+i, y+j, r, g, b, a*alpha);
		}
	}
}

void Graphics::draw_image(pixel *img, int x, int y, int w, int h, int a)
{
	int i, j, r, g, b;
	if (!img) return;
	if(y + h > YRES+MENUSIZE) h = (YRES+MENUSIZE)-y; //Adjust height to prevent drawing off the bottom
	if(a >= 255)
		for (j=0; j<h; j++)
			for (i=0; i<w; i++)
			{
				vid[(y+j)*(XRES+BARSIZE)+(x+i)] = *img;
				img++;
			}
	else
		for (j=0; j<h; j++)
			for (i=0; i<w; i++)
			{
				r = PIXR(*img);
				g = PIXG(*img);
				b = PIXB(*img);
				drawpixel(x+i, y+j, r, g, b, a);
				img++;
			}
}

void Graphics::dim_copy(pixel *dst, pixel *src) //old persistent, unused
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

void Graphics::dim_copy_pers(pixel *dst, pixel *src) //for persistent view, reduces rgb slowly
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

/*void render_zoom(pixel *img) //draws the zoom box
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
}*/

/*int render_thumb(void *thumb, int size, int bzip2, pixel *vid_buf, int px, int py, int scl)
{
	unsigned char *d, *c = (unsigned char *)thumb;
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
		d = (unsigned char *)malloc(i);
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
}*/

//draws the cursor
/*void Graphics::render_cursor(pixel *vid, int x, int y, int t, int rx, int ry)
{
#ifdef OGLR
	int i;
	if (t<PT_NUM||(t&0xFF)==PT_LIFE||t==SPC_AIR||t==SPC_HEAT||t==SPC_COOL||t==SPC_VACUUM||t==SPC_WIND||t==SPC_PGRV||t==SPC_NGRV)
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
			glVertex2f(x-rx+1, (y)-ry+1);
			glVertex2f(x+rx+1, (y)-ry+1);
			glVertex2f(x+rx+1, (y)+ry+1);
			glVertex2f(x-rx+1, (y)+ry+1);
			glVertex2f(x-rx+1, (y)-ry+1);
		}
		else if (CURRENT_BRUSH==CIRCLE_BRUSH)
		{
			for (i = 0; i < 360; i++)
			{
			  float degInRad = i*(M_PI/180.0f);
			  glVertex2f((cos(degInRad)*rx)+x, (sin(degInRad)*ry)+y);
			}
		}
		else if (CURRENT_BRUSH==TRI_BRUSH)
		{
			glVertex2f(x+1, (y)+ry+1);
			glVertex2f(x+rx+1, (y)-ry+1);
			glVertex2f(x-rx+1, (y)-ry+1);
			glVertex2f(x+1, (y)+ry+1);
		}
		glEnd();
		glDisable(GL_COLOR_LOGIC_OP);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
#else
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
#endif
}*/

/*int sdl_opened = 0;
int sdl_open(void)
{
	int status;
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
	atexit(SDL_Quit);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES, YRES, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &partsTFY);
		glBindTexture(GL_TEXTURE_2D, partsTFY);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES, YRES, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

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
int draw_debug_info(pixel* vid, Simulation * sim, int lm, int lx, int ly, int cx, int cy, int line_x, int line_y)
{
	Particle * parts = sim->parts;
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
		sprintf(infobuf, "%d/%d (%.2f%%)", sim->parts_lastActiveIndex, NPART, (((float)sim->parts_lastActiveIndex)/((float)NPART))*100.0f);
		for(i = 0; i < NPART; i++){
			if(parts[i].type){
				drawpixel(vid, x, y, 255, 255, 255, 180);
			} else {
				drawpixel(vid, x, y, 0, 0, 0, 180);
			}
			if(i == sim->parts_lastActiveIndex)
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
}*/

void Graphics::Clear()
{
	memset(vid, 0, PIXELSIZE * ((XRES+BARSIZE) * (YRES+MENUSIZE)));
}

void Graphics::AttachSDLSurface(SDL_Surface * surface)
{
	sdl_scrn = surface;
}

void Graphics::Blit()
{
	if(sdl_scrn)
	{
		pixel * dst;
		pixel * src = vid;
		int j, x = 0, y = 0, w = XRES+BARSIZE, h = YRES+MENUSIZE, pitch = XRES+BARSIZE;
		if (SDL_MUSTLOCK(sdl_scrn))
			if (SDL_LockSurface(sdl_scrn)<0)
				return;
		dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
		for (j=0; j<h; j++)
		{
			memcpy(dst, src, w*PIXELSIZE);
			dst+=sdl_scrn->pitch/PIXELSIZE;
			src+=pitch;
		}
		if (SDL_MUSTLOCK(sdl_scrn))
			SDL_UnlockSurface(sdl_scrn);
		SDL_UpdateRect(sdl_scrn,0,0,0,0);
	}
}

Graphics::Graphics():
		sdl_scrn(NULL)
{
	vid = (pixel *)malloc(PIXELSIZE * ((XRES+BARSIZE) * (YRES+MENUSIZE)));
}
Graphics::~Graphics()
{
	free(vid);
}
