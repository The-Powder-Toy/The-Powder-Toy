#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <png.h>
#include "common/platform/Platform.h"
#include "FontReader.h"
#include "Graphics.h"
#include "resampler/resampler.h"
#include "SimulationConfig.h"
#include "RasterDrawMethodsImpl.h"

VideoBuffer::VideoBuffer(Vec2<int> size):
	video(size)
{}

VideoBuffer::VideoBuffer(pixel const *data, Vec2<int> size):
	VideoBuffer(size)
{
	std::copy_n(data, size.X * size.Y, video.data());
}

VideoBuffer::VideoBuffer(pixel const *data, Vec2<int> size, size_t rowStride):
	VideoBuffer(size)
{
	for(int y = 0; y < size.Y; y++)
		std::copy_n(data + rowStride * y, size.X, video.RowIterator(Vec2(0, y)));
}

VideoBuffer::VideoBuffer(int width, int height):
	VideoBuffer(Vec2(width, height))
{}

VideoBuffer::VideoBuffer(VideoBuffer * old):
	VideoBuffer(*old)
{}

VideoBuffer::VideoBuffer(pixel const *buffer, int width, int height, int pitch):
	VideoBuffer(buffer, Vec2(width, height), pitch == 0 ? width : pitch)
{}

void VideoBuffer::Crop(Rect<int> rect)
{
	rect &= Size().OriginRect();
	if (rect == Size().OriginRect())
		return;

	PlaneAdapter<std::vector<pixel> &> newVideo(rect.Size(), video.Base);
	for (auto y = 0; y < newVideo.Size().Y; y++)
		std::copy_n(
			video.RowIterator(rect.TopLeft + Vec2(0, y)),
			newVideo.Size().X,
			newVideo.RowIterator(Vec2(0, y))
		);
	newVideo.Base.resize(newVideo.Size().X * newVideo.Size().Y);
	newVideo.Base.shrink_to_fit();
	video.SetSize(newVideo.Size());
}

void VideoBuffer::Resize(Vec2<int> size, bool resample)
{
	if (size == Size())
		return;

	if (resample)
	{
		std::array<std::unique_ptr<Resampler>, PIXELCHANNELS> resamplers;
		Resampler::Contrib_List *clist_x = NULL, *clist_y = NULL;
		for (auto &ptr : resamplers)
		{
			ptr = std::make_unique<Resampler>(
				Size().X, Size().Y, // source size
				size.X, size.Y, // destination size
				Resampler::BOUNDARY_CLAMP,
				0.0f, 255.0f, // upper and lower bounds for channel values
				"lanczos12",
				clist_x, clist_y,
				0.75f, 0.75f // X and Y filter scales, values < 1.0 cause aliasing, but create sharper looking mips.
			);
			clist_x = ptr->get_clist_x();
			clist_y = ptr->get_clist_y();
		}

		std::array<std::unique_ptr<float []>, PIXELCHANNELS> samples;
		for (auto &ptr : samples)
			ptr = std::make_unique<float []>(Size().X);

		PlaneAdapter<std::vector<pixel>> newVideo(size);

		pixel const *inIter = video.data();
		std::array<pixel *, PIXELCHANNELS> outIter;
		for (pixel *&it : outIter)
			it = newVideo.data();

		for (int sourceY = 0; sourceY < Size().Y; sourceY++)
		{
			for (int sourceX = 0; sourceX < Size().X; sourceX++)
			{
				pixel px = *inIter++;
				for (int c = 0; c < PIXELCHANNELS; c++)
					samples[c][sourceX] = uint8_t(px >> (8 * c));
			}

			for (int c = 0; c < PIXELCHANNELS; c++)
			{
				if (!resamplers[c]->put_line(samples[c].get()))
				{
					fprintf(stderr, "Out of memory when resampling\n");
					Crop(size.OriginRect()); // Better than leaving the image at original size I guess
					return;
				}

				while (float const *output = resamplers[c]->get_line())
					for (int destX = 0; destX < size.X; destX++)
						*outIter[c]++ |= pixel(uint8_t(output[destX])) << (8 * c);
			}
		}

		video = std::move(newVideo);
	}
	else
	{
		PlaneAdapter<std::vector<pixel>> newVideo(size);
		for (auto pos : size.OriginRect())
		{
			auto oldPos = Vec2(pos.X * Size().X / size.X, pos.Y * Size().Y / size.Y);
			newVideo[pos] = video[oldPos];
		}
		video = std::move(newVideo);
	}
}

void VideoBuffer::Resize(float factor, bool resample)
{
	Resize(Vec2<int>(Size() * factor), resample);
}

void VideoBuffer::ResizeToFit(Vec2<int> bound, bool resample)
{
	Vec2<int> size = Size();
	if (size.X > bound.X || size.Y > bound.Y)
	{
		if (bound.X * size.Y < bound.Y * size.X)
			size = size * bound.X / size.X;
		else
			size = size * bound.Y / size.Y;
	}
	Resize(size, resample);
}

int VideoBuffer::SetCharacter(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	// Technically inaccurate but oh well
	return x + BlendChar(Vec2(x, y), c, RGBA<uint8_t>(r, g, b, a));
}

int VideoBuffer::BlendCharacter(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	return x + BlendChar(Vec2(x, y), c, RGBA<uint8_t>(r, g, b, a));
}

int VideoBuffer::AddCharacter(int x, int y, String::value_type c, int r, int g, int b, int a)
{
	return x + AddChar(Vec2(x, y), c, RGBA<uint8_t>(r, g, b, a));
}

template class RasterDrawMethods<VideoBuffer>;

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
	VideoBuffer newBuffer(WINDOW);
	std::copy_n(video.data(), WINDOW.X * WINDOW.Y, newBuffer.Data());
	return newBuffer;
}

void Graphics::SwapClipRect(Rect<int> &rect)
{
	std::swap(clipRect, rect);
	clipRect &= video.Size().OriginRect();
}

void Graphics::SetClipRect(int &x, int &y, int &w, int &h)
{
	Rect<int> rect = RectSized(Vec2(x, y), Vec2(w, h));
	SwapClipRect(rect);
	x = rect.TopLeft.X;
	y = rect.TopLeft.Y;
	w = rect.Size().X;
	h = rect.Size().Y;
}

bool VideoBuffer::WritePNG(const ByteString &path) const
{
	std::vector<png_const_bytep> rowPointers(Height);
	for (auto y = 0; y < Height; ++y)
	{
		rowPointers[y] = (png_const_bytep)&Buffer[y * Width];
	}
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		std::cerr << "WritePNG: png_create_write_struct failed" << std::endl;
		return false;
	}
	png_infop info = png_create_info_struct(png);
	if (!info)
	{
		std::cerr << "WritePNG: png_create_info_struct failed" << std::endl;
		png_destroy_write_struct(&png, (png_infopp)NULL);
		return false;
	}
	if (setjmp(png_jmpbuf(png)))
	{
		// libpng longjmp'd here in its infinite widsom, clean up and return
		std::cerr << "WritePNG: longjmp from within libpng" << std::endl;
		png_destroy_write_struct(&png, &info);
		return false;
	}
	struct InMemoryFile
	{
		std::vector<char> data;
	} imf;
	png_set_write_fn(png, (png_voidp)&imf, [](png_structp png, png_bytep data, size_t length) -> void {
		auto ud = png_get_io_ptr(png);
		auto &imf = *(InMemoryFile *)ud;
		imf.data.insert(imf.data.end(), data, data + length);
	}, NULL);
	png_set_IHDR(png, info, Width, Height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	png_set_filler(png, 0, PNG_FILLER_AFTER);
	png_set_bgr(png);
	png_write_image(png, (png_bytepp)&rowPointers[0]);
	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);
	return Platform::WriteFile(imf.data, path);
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
