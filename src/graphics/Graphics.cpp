#include "Graphics.h"

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include <bzlib.h>
#include "common/Platform.h"

#include "FontReader.h"
#ifdef HIGH_QUALITY_RESAMPLE
#include "resampler/resampler.h"
#endif

#include <png.h>

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

VideoBuffer::VideoBuffer(pixel * buffer, int width, int height, int pitch):
	Width(width),
	Height(height)
{
	Buffer = new pixel[width*height];
	CopyData(buffer, width, height, pitch ? pitch : width);
}

void VideoBuffer::CopyData(pixel * buffer, int width, int height, int pitch)
{
	for (auto y = 0; y < height; ++y)
	{
		std::copy(buffer + y * pitch, buffer + y * pitch + width, Buffer + y * width);
	}
}

void VideoBuffer::Crop(int width, int height, int x, int y)
{
	CopyData(Buffer + y * Width + x, width, height, Width);
	Width = width;
	Height = height;
}

void VideoBuffer::Resize(float factor, bool resample)
{
	int newWidth = int(Width * factor);
	int newHeight = int(Height * factor);
	Resize(newWidth, newHeight, resample);
}

void VideoBuffer::Resize(int width, int height, bool resample, bool fixedRatio)
{
	int newWidth = width;
	int newHeight = height;
	pixel * newBuffer;
	if(newHeight == -1 && newWidth == -1)
		return;
	if(newHeight == -1 || newWidth == -1)
	{
		if(newHeight == -1)
			newHeight = int(float(Height) * newWidth / Width);
		if(newWidth == -1)
			newWidth = int(float(Width) * newHeight / Height);
	}
	else if(fixedRatio)
	{
		//Force proportions
		if(newWidth*Height > newHeight*Width) // same as nW/W > nH/H
			newWidth = (int)(Width * (newHeight/(float)Height));
		else
			newHeight = (int)(Height * (newWidth/(float)Width));
	}
	if(resample)
		newBuffer = Graphics::resample_img(Buffer, Width, Height, newWidth, newHeight);
	else
		newBuffer = Graphics::resample_img_nn(Buffer, Width, Height, newWidth, newHeight);

	if(newBuffer)
	{
		delete[] Buffer;
		Buffer = newBuffer;
		Width = newWidth;
		Height = newHeight;
	}
}

int VideoBuffer::SetCharacter(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			SetPixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

int VideoBuffer::BlendCharacter(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			BlendPixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

int VideoBuffer::AddCharacter(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	FontReader reader(c);
	for (int j = -2; j < FONT_H - 2; j++)
		for (int i = 0; i < reader.GetWidth(); i++)
			AddPixel(x + i, y + j, r, g, b, reader.NextPixel() * a / 3);
	return x + reader.GetWidth();
}

VideoBuffer::~VideoBuffer()
{
	delete[] Buffer;
}

pixel *Graphics::resample_img_nn(pixel * src, int sw, int sh, int rw, int rh)
{
	int y, x;
	pixel *q = NULL;
	q = new pixel[rw*rh];
	for (y=0; y<rh; y++)
		for (x=0; x<rw; x++){
			q[rw*y+x] = src[sw*(y*sh/rh)+(x*sw/rw)];
		}
	return q;
}

pixel *Graphics::resample_img(pixel *src, int sw, int sh, int rw, int rh)
{
#ifdef HIGH_QUALITY_RESAMPLE

	unsigned char * source = (unsigned char*)src;
	int sourceWidth = sw, sourceHeight = sh;
	int resultWidth = rw, resultHeight = rh;
	int sourcePitch = sourceWidth*PIXELSIZE, resultPitch = resultWidth*PIXELSIZE;
	// Filter scale - values < 1.0 cause aliasing, but create sharper looking mips.
	const float filter_scale = 0.75f;
	const char* pFilter = "lanczos12";


	Resampler * resamplers[PIXELCHANNELS];
	float * samples[PIXELCHANNELS];

	//Resampler for each colour channel
	if (sourceWidth <= 0 || sourceHeight <= 0 || resultWidth <= 0 || resultHeight <= 0)
		return NULL;
	resamplers[0] = new Resampler(sourceWidth, sourceHeight, resultWidth, resultHeight, Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f, pFilter, NULL, NULL, filter_scale, filter_scale);
	samples[0] = new float[sourceWidth];
	for (int i = 1; i < PIXELCHANNELS; i++)
	{
		resamplers[i] = new Resampler(sourceWidth, sourceHeight, resultWidth, resultHeight, Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f, pFilter, resamplers[0]->get_clist_x(), resamplers[0]->get_clist_y(), filter_scale, filter_scale);
		samples[i] = new float[sourceWidth];
	}

	unsigned char * resultImage = new unsigned char[resultHeight * resultPitch];
	std::fill(resultImage, resultImage + (resultHeight*resultPitch), 0);

	//Resample time
	int resultY = 0;
	for (int sourceY = 0; sourceY < sourceHeight; sourceY++)
	{
		unsigned char * sourcePixel = &source[sourceY * sourcePitch];

		//Move pixel components into channel samples
		for (int c = 0; c < PIXELCHANNELS; c++)
		{
			for (int x = 0; x < sourceWidth; x++)
			{
				samples[c][x] = sourcePixel[(x*PIXELSIZE)+c] * (1.0f/255.0f);
			}
		}

		//Put channel sample data into resampler
		for (int c = 0; c < PIXELCHANNELS; c++)
		{
			if (!resamplers[c]->put_line(&samples[c][0]))
			{
				printf("Out of memory!\n");
				return NULL;
			}
		}

		//Perform resample and Copy components from resampler result samples to image buffer
		for ( ; ; )
		{
			int comp_index;
			for (comp_index = 0; comp_index < PIXELCHANNELS; comp_index++)
			{
				const float* resultSamples = resamplers[comp_index]->get_line();
				if (!resultSamples)
					break;

				unsigned char * resultPixel = &resultImage[(resultY * resultPitch) + comp_index];

				for (int x = 0; x < resultWidth; x++)
				{
					int c = (int)(255.0f * resultSamples[x] + .5f);
					if (c < 0) c = 0; else if (c > 255) c = 255;
					*resultPixel = (unsigned char)c;
					resultPixel += PIXELSIZE;
				}
			}
			if (comp_index < PIXELCHANNELS)
				break;

			resultY++;
		}
	}

	//Clean up
	for(int i = 0; i < PIXELCHANNELS; i++)
	{
		delete resamplers[i];
		delete[] samples[i];
	}

	return (pixel*)resultImage;
#else
#ifdef DEBUG
	std::cout << "Resampling " << sw << "x" << sh << " to " << rw << "x" << rh << std::endl;
#endif
	bool stairstep = false;
	if(rw < sw || rh < sh)
	{
		float fx = (float)(((float)sw)/((float)rw));
		float fy = (float)(((float)sh)/((float)rh));

		int fxint, fyint;
		double fxintp_t, fyintp_t;

		float fxf = modf(fx, &fxintp_t), fyf = modf(fy, &fyintp_t);
		fxint = fxintp_t;
		fyint = fyintp_t;

		if(((fxint & (fxint-1)) == 0 && fxf < 0.1f) || ((fyint & (fyint-1)) == 0 && fyf < 0.1f))
			stairstep = true;

#ifdef DEBUG
		if(stairstep)
			std::cout << "Downsampling by " << fx << "x" << fy << " using stairstepping" << std::endl;
		else
			std::cout << "Downsampling by " << fx << "x" << fy << " without stairstepping" << std::endl;
#endif
	}

	int y, x, fxceil, fyceil;
	//int i,j,x,y,w,h,r,g,b,c;
	pixel *q = NULL;
	if(rw == sw && rh == sh){
		//Don't resample
		q = new pixel[rw*rh];
		std::copy(src, src+(rw*rh), q);
	} else if(!stairstep) {
		float fx, fy, fyc, fxc;
		double intp;
		pixel tr, tl, br, bl;
		q = new pixel[rw*rh];
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
		oq = new pixel[sw*sh];
		std::copy(src, src+(sw*sh), oq);
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
			q = new pixel[rw*rh];
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
			delete[] oq;
			oq = q;
			sw = rw;
			sh = rh;
		}
	}
	return q;
#endif
}

int Graphics::textwidth(const String &str)
{
	int x = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\b')
		{
			if (str.length() <= i+1)
				break;
			i++;
			continue;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i+3)
				break;
			i += 3;
			continue;
		}
		x += FontReader(str[i]).GetWidth();
	}
	return x-1;
}

int Graphics::CharWidth(String::value_type c)
{
	return FontReader(c).GetWidth();
}

int Graphics::textwidthx(const String &str, int w)
{
	int x = 0,n = 0,cw = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\b')
		{
			if (str.length() <= i+1)
				break;
			i++;
			continue;
		} else if (str[i] == '\x0F') {
			if (str.length() <= i+3)
				break;
			i += 3;
			continue;
		}
		cw = FontReader(str[i]).GetWidth();
		if (x+(cw/2) >= w)
			break;
		x += cw;
		n++;
	}
	return n;
}

void Graphics::textsize(const String &str, int & width, int & height)
{
	if(!str.size())
	{
		width = 0;
		height = FONT_H-2;
		return;
	}

	int cHeight = FONT_H-2, cWidth = 0, lWidth = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			cWidth = 0;
			cHeight += FONT_H;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i+3)
				break;
			i += 3;
		}
		else if (str[i] == '\b')
		{
			if (str.length() <= i+1)
				break;
			i++;
		}
		else
		{
			cWidth += FontReader(str[i]).GetWidth();
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
			drawchar(x, y, 0xE001, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0xE001, 255, 255, 255, alpha);
		break;
	case IconReload:
		if(invert)
			drawchar(x, y, 0xE011, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0xE011, 255, 255, 255, alpha);
		break;
	case IconSave:
		if(invert)
			drawchar(x, y, 0xE002, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0xE002, 255, 255, 255, alpha);
		break;
	case IconVoteUp:
		if(invert)
		{
			drawchar(x-11, y+1, 0xE04B, 0, 100, 0, alpha);
			drawtext(x+2, y+1, "Vote", 0, 100, 0, alpha);
		}
		else
		{
			drawchar(x-11, y+1, 0xE04B, 0, 187, 18, alpha);
			drawtext(x+2, y+1, "Vote", 0, 187, 18, alpha);
		}
		break;
	case IconVoteDown:
		if(invert)
			drawchar(x, y, 0xE04A, 100, 10, 0, alpha);
		else
			drawchar(x, y, 0xE04A, 187, 40, 0, alpha);
		break;
	case IconTag:
		if(invert)
			drawchar(x, y, 0xE003, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0xE003, 255, 255, 255, alpha);
		break;
	case IconNew:
		if(invert)
			drawchar(x, y, 0xE012, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0xE012, 255, 255, 255, alpha);
		break;
	case IconLogin:
		if(invert)
			drawchar(x, y+1, 0xE004, 0, 0, 0, alpha);
		else
			drawchar(x, y+1, 0xE004, 255, 255, 255, alpha);
		break;
	case IconSimulationSettings:
		if(invert)
			drawchar(x, y+1, 0xE04F, 0, 0, 0, alpha);
		else
			drawchar(x, y+1, 0xE04F, 255, 255, 255, alpha);
		break;
	case IconRenderSettings:
		if(invert)
		{
			drawchar(x, y+1, 0xE058, 255, 0, 0, alpha);
			drawchar(x, y+1, 0xE059, 0, 255, 0, alpha);
			drawchar(x, y+1, 0xE05A, 0, 0, 255, alpha);
		}
		else
		{
			addchar(x, y+1, 0xE058, 255, 0, 0, alpha);
			addchar(x, y+1, 0xE059, 0, 255, 0, alpha);
			addchar(x, y+1, 0xE05A, 0, 0, 255, alpha);
		}
		break;
	case IconPause:
		if(invert)
			drawchar(x, y, 0xE010, 0, 0, 0, alpha);
		else
			drawchar(x, y, 0xE010, 255, 255, 255, alpha);
		break;
	case IconFavourite:
		if(invert)
			drawchar(x, y, 0xE04C, 100, 80, 32, alpha);
		else
			drawchar(x, y, 0xE04C, 192, 160, 64, alpha);
		break;
	case IconReport:
		if(invert)
			drawchar(x, y, 0xE063, 140, 140, 0, alpha);
		else
			drawchar(x, y, 0xE063, 255, 255, 0, alpha);
		break;
	case IconUsername:
		if(invert)
		{
			drawchar(x, y, 0xE00B, 32, 64, 128, alpha);
			drawchar(x, y, 0xE00A, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0xE00B, 32, 64, 128, alpha);
			drawchar(x, y, 0xE00A, 255, 255, 255, alpha);
		}
		break;
	case IconPassword:
		if(invert)
		{
			drawchar(x, y, 0xE00C, 160, 144, 32, alpha);
			drawchar(x, y, 0xE004, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0xE00C, 160, 144, 32, alpha);
			drawchar(x, y, 0xE004, 255, 255, 255, alpha);
		}
		break;
	case IconClose:
		if(invert)
			drawchar(x, y, 0xE02A, 20, 20, 20, alpha);
		else
			drawchar(x, y, 0xE02A, 230, 230, 230, alpha);
		break;
	case IconVoteSort:
		if (invert)
		{
			drawchar(x, y, 0xE029, 44, 48, 32, alpha);
			drawchar(x, y, 0xE028, 32, 44, 32, alpha);
			drawchar(x, y, 0xE027, 128, 128, 128, alpha);
		}
		else
		{
			drawchar(x, y, 0xE029, 144, 48, 32, alpha);
			drawchar(x, y, 0xE028, 32, 144, 32, alpha);
			drawchar(x, y, 0xE027, 255, 255, 255, alpha);
		}
		break;
	case IconDateSort:
		if (invert)
		{
			drawchar(x, y, 0xE026, 32, 32, 32, alpha);
		}
		else
		{
			drawchar(x, y, 0xE026, 255, 255, 255, alpha);
		}
		break;
	case IconMyOwn:
		if (invert)
		{
			drawchar(x, y, 0xE014, 192, 160, 64, alpha);
			drawchar(x, y, 0xE013, 32, 32, 32, alpha);
		}
		else
		{
			drawchar(x, y, 0xE014, 192, 160, 64, alpha);
			drawchar(x, y, 0xE013, 255, 255, 255, alpha);
		}
		break;
	case IconSearch:
		drawchar(x, y, 0xE00E, 30, 30, 180, alpha);
		drawchar(x, y, 0xE00F, 255, 255, 255, alpha);
		break;
	case IconDelete:
		if(invert)
		{
			drawchar(x, y, 0xE006, 159, 47, 31, alpha);
			drawchar(x, y, 0xE005, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0xE006, 159, 47, 31, alpha);
			drawchar(x, y, 0xE005, 255, 255, 255, alpha);
		}
		break;
	case IconAdd:
		if(invert)
		{
			drawchar(x, y, 0xE006, 32, 144, 32, alpha);
			drawchar(x, y, 0xE009, 0, 0, 0, alpha);
		}
		else
		{
			drawchar(x, y, 0xE006, 32, 144, 32, alpha);
			drawchar(x, y, 0xE009, 255, 255, 255, alpha);
		}
		break;
	case IconVelocity:
		drawchar(x+1, y, 0xE018, 128, 160, 255, alpha);
		break;
	case IconPressure:
		if(invert)
			drawchar(x+1, y+1, 0xE019, 180, 160, 16, alpha);
		else
			drawchar(x+1, y+1, 0xE019, 255, 212, 32, alpha);
		break;
	case IconPersistant:
		if(invert)
			drawchar(x+1, y+1, 0xE01A, 20, 20, 20, alpha);
		else
			drawchar(x+1, y+1, 0xE01A, 212, 212, 212, alpha);
		break;
	case IconFire:
		drawchar(x+1, y+1, 0xE01B, 255, 0, 0, alpha);
		drawchar(x+1, y+1, 0xE01C, 255, 255, 64, alpha);
		break;
	case IconBlob:
		if(invert)
			drawchar(x+1, y, 0xE03F, 55, 180, 55, alpha);
		else
			drawchar(x+1, y, 0xE03F, 55, 255, 55, alpha);
		break;
	case IconHeat:
		drawchar(x+3, y, 0xE03E, 255, 0, 0, alpha);
		if(invert)
			drawchar(x+3, y, 0xE03D, 0, 0, 0, alpha);
		else
			drawchar(x+3, y, 0xE03D, 255, 255, 255, alpha);
		break;
	case IconBlur:
		if(invert)
			drawchar(x+1, y, 0xE044, 50, 70, 180, alpha);
		else
			drawchar(x+1, y, 0xE044, 100, 150, 255, alpha);
		break;
	case IconGradient:
		if(invert)
			drawchar(x+1, y+1, 0xE053, 255, 50, 255, alpha);
		else
			drawchar(x+1, y+1, 0xE053, 205, 50, 205, alpha);
		break;
	case IconLife:
		if(invert)
			drawchar(x, y+1, 0xE060, 0, 0, 0, alpha);
		else
			drawchar(x, y+1, 0xE060, 255, 255, 255, alpha);
		break;
	case IconEffect:
		drawchar(x+1, y, 0xE061, 255, 255, 160, alpha);
		break;
	case IconGlow:
		drawchar(x+1, y, 0xE05F, 200, 255, 255, alpha);
		break;
	case IconWarp:
		drawchar(x+1, y, 0xE05E, 255, 255, 255, alpha);
		break;
	case IconBasic:
		if(invert)
			drawchar(x+1, y+1, 0xE05B, 50, 50, 0, alpha);
		else
			drawchar(x+1, y+1, 0xE05B, 255, 255, 200, alpha);
		break;
	case IconAltAir:
		if(invert) {
			drawchar(x+1, y+1, 0xE054, 180, 55, 55, alpha);
			drawchar(x+1, y+1, 0xE055, 55, 180, 55, alpha);
		} else {
			drawchar(x+1, y+1, 0xE054, 255, 55, 55, alpha);
			drawchar(x+1, y+1, 0xE055, 55, 255, 55, alpha);
		}
		break;
	default:
		if(invert)
			drawchar(x, y, 't', 0, 0, 0, alpha);
		else
			drawchar(x, y, 't', 255, 255, 255, alpha);
		break;
	}
}

void Graphics::draw_rgba_image(const pixel *data, int w, int h, int x, int y, float alpha)
{
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			auto rgba = *(data++);
			auto a = (rgba >> 24) & 0xFF;
			auto r = (rgba >> 16) & 0xFF;
			auto g = (rgba >>  8) & 0xFF;
			auto b = (rgba      ) & 0xFF;
			addpixel(x+i, y+j, r, g, b, (int)(a*alpha));
		}
	}
}

VideoBuffer Graphics::DumpFrame()
{
	VideoBuffer newBuffer(WINDOWW, WINDOWH);
	std::copy(vid, vid+(WINDOWW*WINDOWH), newBuffer.Buffer);
	return newBuffer;
}

void Graphics::SetClipRect(int &x, int &y, int &w, int &h)
{
	int newX = x;
	int newY = y;
	int newW = w;
	int newH = h;
	if (newX < 0) newX = 0;
	if (newY < 0) newY = 0;
	if (newW > WINDOWW - newX) newW = WINDOWW - newX;
	if (newH > WINDOWH - newY) newH = WINDOWH - newY;
	x = clipx1;
	y = clipy1;
	w = clipx2 - clipx1;
	h = clipy2 - clipy1;
	clipx1 = newX;
	clipy1 = newY;
	clipx2 = newX + newW;
	clipy2 = newY + newH;
}

bool VideoBuffer::WritePNG(const ByteString &path) const
{
	std::vector<png_const_bytep> rowPointers(Height);
	for (auto y = 0; y < Height; ++y)
	{
		rowPointers[y] = (png_const_bytep)&Buffer[y * Width];
	}
#ifdef WIN
	FILE *f = _wfopen(Platform::WinWiden(path).c_str(), L"wb");
#else
	FILE *f = fopen(path.c_str(), "wb");
#endif
	if (!f)
	{
		std::cerr << "WritePNG: fopen failed" << std::endl;
		return false;
	}
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		std::cerr << "WritePNG: png_create_write_struct failed" << std::endl;
		fclose(f);
		return false;
	}
	png_infop info = png_create_info_struct(png);
	if (!info)
	{
		std::cerr << "WritePNG: png_create_info_struct failed" << std::endl;
		png_destroy_write_struct(&png, (png_infopp)NULL);
		fclose(f);
		return false;
	}
	if (setjmp(png_jmpbuf(png)))
	{
		// libpng longjmp'd here in its infinite widsom, clean up and return
		std::cerr << "WritePNG: longjmp from within libpng" << std::endl;
		png_destroy_write_struct(&png, &info);
		fclose(f);
		return false;
	}
	png_init_io(png, f);
	png_set_IHDR(png, info, Width, Height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	png_set_filler(png, 0, PNG_FILLER_AFTER);
	png_set_bgr(png);
	png_write_image(png, (png_bytepp)&rowPointers[0]);
	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);
	fclose(f);
	return true;
}

bool PngDataToPixels(std::vector<pixel> &imageData, int &imgw, int &imgh, const char *pngData, size_t pngDataSize, bool addBackground)
{
	std::vector<png_const_bytep> rowPointers;
	struct InMemoryFile
	{
		png_const_bytep data;
		size_t size;
		size_t cursor;
	} imf{ (png_const_bytep)pngData, pngDataSize, 0 };
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		std::cerr << "pngDataToPixels: png_create_read_struct failed" << std::endl;
		return false;
	}
	png_infop info = png_create_info_struct(png);
	if (!info)
	{
		std::cerr << "pngDataToPixels: png_create_info_struct failed" << std::endl;
		png_destroy_read_struct(&png, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}
	if (setjmp(png_jmpbuf(png)))
	{
		// libpng longjmp'd here in its infinite widsom, clean up and return
		std::cerr << "pngDataToPixels: longjmp from within libpng" << std::endl;
		png_destroy_read_struct(&png, &info, (png_infopp)NULL);
		return false;
	}
	png_set_read_fn(png, (png_voidp)&imf, [](png_structp png, png_bytep data, size_t length) -> void {
		auto ud = png_get_io_ptr(png);
		auto &imf = *(InMemoryFile *)ud;
		if (length + imf.cursor > imf.size)
		{
			png_error(png, "pngDataToPixels: libpng tried to read beyond the buffer");
		}
		std::copy(imf.data + imf.cursor, imf.data + imf.cursor + length, data);
		imf.cursor += length;
	});
	png_set_user_limits(png, 1000, 1000);
	png_read_info(png, info);
	imgw = png_get_image_width(png, info);
	imgh = png_get_image_height(png, info);
	int bitDepth = png_get_bit_depth(png, info);
	int colorType = png_get_color_type(png, info);
	imageData.resize(imgw * imgh);
	rowPointers.resize(imgh);
	for (auto y = 0; y < imgh; ++y)
	{
		rowPointers[y] = (png_const_bytep)&imageData[y * imgw];
	}
	if (setjmp(png_jmpbuf(png)))
	{
		// libpng longjmp'd here in its infinite widsom, clean up and return
		std::cerr << "pngDataToPixels: longjmp from within libpng" << std::endl;
		png_destroy_read_struct(&png, &info, (png_infopp)NULL);
		return false;
	}
	if (addBackground)
	{
		png_set_filler(png, 0, PNG_FILLER_AFTER);
	}
	png_set_bgr(png);
	if (colorType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png);
	}
	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(png);
	}
	if (png_get_valid(png, info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png);
	}
	if (bitDepth == 16)
	{
		png_set_scale_16(png);
	}
	if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png);
	}
	if (addBackground)
	{
		png_color_16 defaultBackground;
		defaultBackground.red = 0;
		defaultBackground.green = 0;
		defaultBackground.blue = 0;
		png_set_background(png, &defaultBackground, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	}
	png_read_image(png, (png_bytepp)&rowPointers[0]);
	png_destroy_read_struct(&png, &info, (png_infopp)NULL);
	return true;
}

bool Graphics::GradientStop::operator <(const GradientStop &other) const
{
	return point < other.point;
}

std::vector<pixel> Graphics::Gradient(std::vector<GradientStop> stops, int resolution)
{
	std::vector<pixel> table(resolution, 0);
	if (stops.size() >= 2)
	{
		std::sort(stops.begin(), stops.end());
		auto stop = -1;
		for (auto i = 0; i < resolution; ++i)
		{
			auto point = i / (float)resolution;
			while (stop < (int)stops.size() - 1 && stops[stop + 1].point <= point)
			{
				++stop;
			}
			if (stop < 0 || stop >= (int)stops.size() - 1)
			{
				continue;
			}
			auto &left = stops[stop];
			auto &right = stops[stop + 1];
			auto f = (point - left.point) / (right.point - left.point);
			table[i] = PIXRGB(
				int(int(PIXR(left.color)) + (int(PIXR(right.color)) - int(PIXR(left.color))) * f),
				int(int(PIXG(left.color)) + (int(PIXG(right.color)) - int(PIXG(left.color))) * f),
				int(int(PIXB(left.color)) + (int(PIXB(right.color)) - int(PIXB(left.color))) * f)
			);
		}
	}
	return table;
}
