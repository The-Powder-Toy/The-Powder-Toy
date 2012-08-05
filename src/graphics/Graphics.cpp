#include <cmath>
#include <iostream>
#include <bzlib.h>
#include <string>
#include "Config.h"
#include "Misc.h"
#include "Graphics.h"
#define INCLUDE_FONTDATA
#include "font.h"

VideoBuffer::VideoBuffer(int width, int height):
	Width(width),
	Height(height)
{
	Buffer = new pixel[width*height];
	std::fill(Buffer, Buffer+(width*height), 0);
};

VideoBuffer::VideoBuffer(const VideoBuffer & old):
	Width(old.Width),
	Height(old.Height)
{
	Buffer = new pixel[old.Width*old.Height];
	std::copy(old.Buffer, old.Buffer+(old.Width*old.Height), Buffer);
};

VideoBuffer::VideoBuffer(VideoBuffer * old):
	Width(old->Width),
	Height(old->Height)
{
	Buffer = new pixel[old->Width*old->Height];
	std::copy(old->Buffer, old->Buffer+(old->Width*old->Height), Buffer);
};

VideoBuffer::~VideoBuffer()
{
	delete[] Buffer;
};

TPT_INLINE void VideoBuffer::BlendPixel(int x, int y, int r, int g, int b, int a)
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

TPT_INLINE void VideoBuffer::SetPixel(int x, int y, int r, int g, int b, int a)
{
	if (x<0 || y<0 || x>=Width || y>=Height)
			return;
#ifdef PIX32OGL
	Buffer[y*(Width)+x] = PIXRGBA(r,g,b,a);
#else
	Buffer[y*(Width)+x] = PIXRGB((r*a)>>8, (g*a)>>8, (b*a)>>8);
#endif
}

TPT_INLINE void VideoBuffer::AddPixel(int x, int y, int r, int g, int b, int a)
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

TPT_INLINE int VideoBuffer::SetCharacter(int x, int y, int c, int r, int g, int b, int a)
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
			SetPixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

TPT_INLINE int VideoBuffer::BlendCharacter(int x, int y, int c, int r, int g, int b, int a)
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
			BlendPixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

TPT_INLINE int VideoBuffer::AddCharacter(int x, int y, int c, int r, int g, int b, int a)
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
			AddPixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

/**
 * Common graphics functions, mostly static methods that provide
 * encoding/decoding of different formats and font metrics
 */

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
#ifdef DEBUG
	std::cout << "Resampling " << sw << "x" << sh << " to " << rw << "x" << rh << std::endl;
#endif
	int y, x, fxceil, fyceil;
	//int i,j,x,y,w,h,r,g,b,c;
	pixel *q = NULL;
	if(rw == sw && rh == sh){
		//Don't resample
		q = (pixel *)malloc(rw*rh*PIXELSIZE);
		memcpy(q, src, rw*rh*PIXELSIZE);
	} else if(rw >= sw && rh >= sh){
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
			if(rw > rrw)
				rw *= 0.7;
			if(rh > rrh)
				rh *= 0.7;
			if(rw <= rrw)
				rw = rrw;
			if(rh <= rrh)
				rh = rrh;
			q = (pixel *)malloc(rw*rh*PIXELSIZE);
			//Bilinear interpolation
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

int Graphics::textwidth(const char *s)
{
	int x = 0;
	for (; *s; s++)
		x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
	return x-1;
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
			if(!s[1]) break;
			s++;
			continue;
		} else if(*s == '\x0F') {
			if(!s[1] || !s[2] || !s[3]) break;
			s+=3;
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
			if(!s[1]) break;
			s++;
			continue;
		} else if (*s == '\x0F')
		{
			if(!s[1] || !s[2] || !s[3]) break;
			s+=3;
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

int Graphics::PositionAtCharIndex(char *s, int charIndex, int & positionX, int & positionY)
{
	int x = 0, y = 0, lines = 1;
	for (; *s; s++)
	{
		if (!charIndex)
			break;
		if(*s == '\n') {
			lines++;
			x = 0;
			y += FONT_H+2;
			charIndex--;
			continue;
		} else if(*s =='\b') {
			if(!s[1]) break;
			s++;
			charIndex-=2;
			continue;
		} else if(*s == '\x0F') {
			if(!s[1] || !s[2] || !s[3]) break;
			s+=3;
			charIndex-=4;
			continue;
		}
		x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
		charIndex--;
	}
	positionX = x;
	positionY = y;
	return lines;
}

int Graphics::CharIndexAtPosition(char *s, int positionX, int positionY)
{
	int x=0, y=0,charIndex=0,cw;
	for (; *s; s++)
	{
		if(*s == '\n') {
			x = 0;
			y += FONT_H+2;
			charIndex++;
			continue;
		} else if(*s == '\b') {
			if(!s[1]) break;
			s++;
			charIndex+=2;
			continue;
		} else if (*s == '\x0F') {
			if(!s[1] || !s[2] || !s[3]) break;
			s+=3;
			charIndex+=4;
			continue;
		}
		cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
		if ((x+(cw/2) >= positionX && y+FONT_H >= positionY) || y > positionY)
			break;
		x += cw;
		charIndex++;
	}
	return charIndex;
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
				if(!s[1]) break;
				s++;
			}
			else if (*s == '\x0F')
			{
				if(!s[1] || !s[2] || !s[3]) break;
				s+=3;
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

void Graphics::textsize(const char * s, int & width, int & height)
{
	if(!strlen(s))
	{
		width = 0;
		height = FONT_H;
		return;
	}

	int cHeight = FONT_H, cWidth = 0, lWidth = 0;
	for (; *s; s++)
	{
		if (*s == '\n')
		{
			cWidth = 0;
			cHeight += FONT_H+2;
		}
		else if (*s == '\x0F')
		{
			if(!s[1] || !s[2] || !s[1]) break;
			s+=3;
		}
		else if (*s == '\b')
		{
			if(!s[1]) break;
			s++;
		}
		else
		{
			cWidth += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
			if(cWidth>lWidth)
				lWidth = cWidth;
		}
	}
	width = lWidth;
	height = cHeight;
}

void Graphics::draw_icon(int x, int y, Icon icon, unsigned char alpha, bool invert)
{
	y--;
	switch(icon)
	{
	case IconOpen:
		if(invert)
			drawchar(x, y, 0x81, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x81, 255, 255, 255, alpha);
		break;
	case IconReload:
		if(invert)
			drawchar(x, y, 0x91, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x91, 255, 255, 255, alpha);
		break;
	case IconSave:
		if(invert)
			drawchar(x, y, 0x82, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x82, 255, 255, 255, alpha);
		break;
	case IconVoteUp:
		if(invert)
			drawchar(x, y, 0xCB, 0, 100, 0, alpha);
		else
			drawchar(x, y, 0xCB, 0, 187, 18, alpha);
		break;
	case IconVoteDown:
		if(invert)
			drawchar(x, y, 0xCA, 100, 10, 0, alpha);
		else
			drawchar(x, y, 0xCA, 187, 40, 0, alpha);
		break;
	case IconTag:
		if(invert)
			drawchar(x, y, 0x83, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x83, 255, 255, 255, alpha);
		break;
	case IconNew:
		if(invert)
			drawchar(x, y, 0x92, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x92, 255, 255, 255, alpha);
		break;
	case IconLogin:
		if(invert)
			drawchar(x, y, 0x84, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x84, 255, 255, 255, alpha);
		break;
	case IconSimulationSettings:
		if(invert)
			drawchar(x, y+1, 0xCF, 0, 0, 0, alpha);
		else
			drawchar(x, y+1, 0xCF, 255, 255, 255, alpha);
		break;
	case IconRenderSettings:
		if(invert)
		{
			drawchar(x, y+1, 0xD8, 255, 0, 0, alpha);
			drawchar(x, y+1, 0xD9, 0, 255, 0, alpha);
			drawchar(x, y+1, 0xDA, 0, 0, 255, alpha);
		}
		else
		{
			addchar(x, y+1, 0xD8, 255, 0, 0, alpha);
			addchar(x, y+1, 0xD9, 0, 255, 0, alpha);
			addchar(x, y+1, 0xDA, 0, 0, 255, alpha);
		}
		break;
	case IconPause:
		if(invert)
			drawchar(x, y, 0x90, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0x90, 255, 255, 255, alpha);
		break;
	case IconFavourite:
		if(invert)
			drawchar(x, y, 0xCC, 100, 80, 32, alpha);
		else
			drawchar(x, y, 0xCC, 192, 160, 64, alpha);
		break;
	case IconReport:
		if(invert)
			drawchar(x, y, 0xE3, 140, 140, 0, alpha);
		else
			drawchar(x, y, 0xE3, 255, 255, 0, alpha);
		break;
	case IconUsername:
		if(invert)
		{
			drawchar(x, y, 0x8B, 32, 64, 128, alpha);
			drawchar(x, y, 0x8A, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0x8B, 32, 64, 128, alpha);
			drawchar(x, y, 0x8A, 255, 255, 255, alpha);
		}
		break;
	case IconPassword:
		if(invert)
		{
			drawchar(x, y, 0x8C, 160, 144, 32, alpha);
			drawchar(x, y, 0x84, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0x8C, 160, 144, 32, alpha);
			drawchar(x, y, 0x84, 255, 255, 255, alpha);
		}
		break;
	case IconClose:
		if(invert)
			drawchar(x, y, 0xAA, 20, 20, 20, alpha);	
		else
			drawchar(x, y, 0xAA, 230, 230, 230, alpha);	
		break;
	case IconVoteSort:
	case IconDateSort:
	case IconFolder:
	case IconSearch:
		drawchar(x, y+1, 0x8E, 30, 30, 180, alpha);
		drawchar(x, y+1, 0x8F, 255, 255, 255, alpha);
		break;
	case IconDelete:
		if(invert)
		{
			drawchar(x, y, 0x86, 255, 55, 55, alpha);
			drawchar(x, y, 0x85, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0x86, 255, 55, 55, alpha);
			drawchar(x, y, 0x85, 255, 255, 255, alpha);
		}
		break;
	default:
		if(invert)
			drawchar(x, y, 't', 0, 0 ,0 ,alpha);
		else
			drawchar(x, y, 't', 255, 255, 255, alpha);
		break;
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

void Graphics::draw_image(const VideoBuffer & vidBuf, int x, int y, int a)
{
	draw_image(vidBuf.Buffer, x, y, vidBuf.Width, vidBuf.Height, a);
}

void Graphics::draw_image(VideoBuffer * vidBuf, int x, int y, int a)
{
	draw_image(vidBuf->Buffer, x, y, vidBuf->Width, vidBuf->Height, a);
}

