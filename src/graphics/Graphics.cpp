#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <png.h>
#include "common/platform/Platform.h"
#include "FontReader.h"
#include "Format.h"
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

void VideoBuffer::Crop(Rect<int> rect)
{
	rect &= Size().OriginRect();
	if (rect == Size().OriginRect())
		return;

	PlaneAdapter<std::vector<pixel> &> newVideo(rect.size, std::in_place, video.Base);
	for (auto y = 0; y < newVideo.Size().Y; y++)
		std::copy_n(
			video.RowIterator(rect.pos + Vec2(0, y)),
			newVideo.Size().X,
			newVideo.RowIterator(Vec2(0, y))
		);
	video.Base.resize(newVideo.Size().X * newVideo.Size().Y);
	video.Base.shrink_to_fit();
	video.SetSize(newVideo.Size());
}

void VideoBuffer::Resize(Vec2<int> size, bool resample)
{
	if (size == Size())
		return;

	if (resample)
	{
		std::array<std::unique_ptr<Resampler>, PIXELCHANNELS> resamplers;
		Resampler::Contrib_List *clist_x = nullptr, *clist_y = nullptr;
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
	Resize(Vec2{ int(Size().X * factor), int(Size().Y * factor) }, resample);
}

void VideoBuffer::ResizeToFit(Vec2<int> bound, bool resample)
{
	Vec2<int> size = Size();
	if (size.X > bound.X || size.Y > bound.Y)
	{
		auto ceilDiv = [](int a, int b) {
			return a / b + ((a % b) ? 1 : 0);
		};
		if (bound.X * size.Y < bound.Y * size.X)
			size = { bound.X, ceilDiv(size.Y * bound.X, size.X) };
		else
			size = { ceilDiv(size.X * bound.Y, size.Y), bound.Y };
	}
	Resize(size, resample);
}

std::unique_ptr<VideoBuffer> VideoBuffer::FromPNG(std::span<const char> data)
{
	auto video = format::PixelsFromPNG(data, 0x000000_rgb);
	if (video)
	{
		auto buf = std::make_unique<VideoBuffer>(Vec2<int>::Zero);
		buf->video = std::move(*video);
		return buf;
	}
	else
		return nullptr;
}

std::unique_ptr<std::vector<char>> VideoBuffer::ToPNG() const
{
	return format::PixelsToPNG(video);
}

std::vector<char> VideoBuffer::ToPPM() const
{
	return format::PixelsToPPM(video);
}

template struct RasterDrawMethods<VideoBuffer>;

Graphics::Graphics()
{}

void Graphics::draw_icon(int x, int y, Icon icon, unsigned char alpha, bool invert)
{
	y--;
	switch(icon)
	{
	case IconOpen:
		if(invert)
			BlendChar({ x, y }, 0xE001, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE001, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconReload:
		if(invert)
			BlendChar({ x, y }, 0xE011, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE011, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconSave:
		if(invert)
			BlendChar({ x, y }, 0xE002, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE002, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconVoteUp:
		if(invert)
		{
			BlendChar({ x-11, y+1 }, 0xE04B, 0x006400_rgb .WithAlpha(alpha));
			BlendText({ x+2, y+1 }, "Vote", 0x006400_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x-11, y+1 }, 0xE04B, 0x00BB12_rgb .WithAlpha(alpha));
			BlendText({ x+2, y+1 }, "Vote", 0x00BB12_rgb .WithAlpha(alpha));
		}
		break;
	case IconVoteDown:
		if(invert)
			BlendChar({ x, y }, 0xE04A, 0x640A00_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE04A, 0xBB2800_rgb .WithAlpha(alpha));
		break;
	case IconTag:
		if(invert)
			BlendChar({ x, y }, 0xE003, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE003, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconNew:
		if(invert)
			BlendChar({ x, y }, 0xE012, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE012, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconLogin:
		if(invert)
			BlendChar({ x, y + 1 }, 0xE004, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y + 1 }, 0xE004, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconSimulationSettings:
		if(invert)
			BlendChar({ x, y + 1 }, 0xE04F, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y + 1 }, 0xE04F, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconRenderSettings:
		if(invert)
		{
			BlendChar({ x, y + 1 }, 0xE058, 0xFF0000_rgb .WithAlpha(alpha));
			BlendChar({ x, y + 1 }, 0xE059, 0x00FF00_rgb .WithAlpha(alpha));
			BlendChar({ x, y + 1 }, 0xE05A, 0x0000FF_rgb .WithAlpha(alpha));
		}
		else
		{
			AddChar({ x, y + 1 }, 0xE058, 0xFF0000_rgb .WithAlpha(alpha));
			AddChar({ x, y + 1 }, 0xE059, 0x00FF00_rgb .WithAlpha(alpha));
			AddChar({ x, y + 1 }, 0xE05A, 0x0000FF_rgb .WithAlpha(alpha));
		}
		break;
	case IconPause:
		if(invert)
			BlendChar({ x, y }, 0xE010, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE010, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconFavourite:
		if(invert)
			BlendChar({ x, y }, 0xE04C, 0x645020_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE04C, 0xC0A040_rgb .WithAlpha(alpha));
		break;
	case IconReport:
		if(invert)
			BlendChar({ x, y }, 0xE063, 0x8C8C00_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE063, 0xFFFF00_rgb .WithAlpha(alpha));
		break;
	case IconUsername:
		if(invert)
		{
			BlendChar({ x, y }, 0xE00B, 0x204080_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE00A, 0x000000_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE00B, 0x204080_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE00A, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconPassword:
		if(invert)
		{
			BlendChar({ x, y }, 0xE00C, 0xA09020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE004, 0x000000_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE00C, 0xA09020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE004, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconClose:
		if(invert)
			BlendChar({ x, y }, 0xE02A, 0x141414_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 0xE02A, 0xE6E6E6_rgb .WithAlpha(alpha));
		break;
	case IconVoteSort:
		if (invert)
		{
			BlendChar({ x, y }, 0xE029, 0x2C3020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE028, 0x202C20_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE027, 0x808080_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE029, 0x903020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE028, 0x209020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE027, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconDateSort:
		if (invert)
		{
			BlendChar({ x, y }, 0xE026, 0x202020_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE026, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconMyOwn:
		if (invert)
		{
			BlendChar({ x, y }, 0xE014, 0xC0A040_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE013, 0x202020_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE014, 0xC0A040_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE013, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconSearch:
		BlendChar({ x, y }, 0xE00E, 0x1E1EB4_rgb .WithAlpha(alpha));
		BlendChar({ x, y }, 0xE00F, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconDelete:
		if(invert)
		{
			BlendChar({ x, y }, 0xE006, 0x9F2F1F_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE005, 0x000000_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE006, 0x9F2F1F_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE005, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconAdd:
		if(invert)
		{
			BlendChar({ x, y }, 0xE006, 0x209020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE009, 0x000000_rgb .WithAlpha(alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE006, 0x209020_rgb .WithAlpha(alpha));
			BlendChar({ x, y }, 0xE009, 0xFFFFFF_rgb .WithAlpha(alpha));
		}
		break;
	case IconVelocity:
		BlendChar({ x + 1, y }, 0xE018, 0x80A0FF_rgb .WithAlpha(alpha));
		break;
	case IconPressure:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE019, 0xB4A010_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE019, 0xFFD420_rgb .WithAlpha(alpha));
		break;
	case IconPersistant:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE01A, 0x141414_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE01A, 0xD4D4D4_rgb .WithAlpha(alpha));
		break;
	case IconFire:
		BlendChar({ x + 1, y + 1 }, 0xE01B, 0xFF0000_rgb .WithAlpha(alpha));
		BlendChar({ x + 1, y + 1 }, 0xE01C, 0xFFFF40_rgb .WithAlpha(alpha));
		break;
	case IconBlob:
		if(invert)
			BlendChar({ x + 1, y }, 0xE03F, 0x37B437_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 1, y }, 0xE03F, 0x37FF37_rgb .WithAlpha(alpha));
		break;
	case IconHeat:
		BlendChar({ x + 3, y }, 0xE03E, 0xFF0000_rgb .WithAlpha(alpha));
		if(invert)
			BlendChar({ x + 3, y }, 0xE03D, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 3, y }, 0xE03D, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconBlur:
		if(invert)
			BlendChar({ x + 1, y }, 0xE044, 0x3246B4_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 1, y }, 0xE044, 0x6496FF_rgb .WithAlpha(alpha));
		break;
	case IconGradient:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE053, 0xFF32FF_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE053, 0xCD32CD_rgb .WithAlpha(alpha));
		break;
	case IconLife:
		if(invert)
			BlendChar({ x, y + 1 }, 0xE060, 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y + 1 }, 0xE060, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconEffect:
		BlendChar({ x + 1, y }, 0xE061, 0xFFFFA0_rgb .WithAlpha(alpha));
		break;
	case IconGlow:
		BlendChar({ x + 1, y }, 0xE05F, 0xC8FFFF_rgb .WithAlpha(alpha));
		break;
	case IconWarp:
		BlendChar({ x + 1, y }, 0xE05E, 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	case IconBasic:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE05B, 0x323200_rgb .WithAlpha(alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE05B, 0xFFFFC8_rgb .WithAlpha(alpha));
		break;
	case IconAltAir:
		if(invert) {
			BlendChar({ x + 1, y + 1 }, 0xE054, 0xB43737_rgb .WithAlpha(alpha));
			BlendChar({ x + 1, y + 1 }, 0xE055, 0x37B437_rgb .WithAlpha(alpha));
		} else {
			BlendChar({ x + 1, y + 1 }, 0xE054, 0xFF3737_rgb .WithAlpha(alpha));
			BlendChar({ x + 1, y + 1 }, 0xE055, 0x37FF37_rgb .WithAlpha(alpha));
		}
		break;
	default:
		if(invert)
			BlendChar({ x, y }, 't', 0x000000_rgb .WithAlpha(alpha));
		else
			BlendChar({ x, y }, 't', 0xFFFFFF_rgb .WithAlpha(alpha));
		break;
	}
}

VideoBuffer Graphics::DumpFrame()
{
	VideoBuffer newBuffer(video.Size());
	std::copy_n(video.data(), video.Size().X * video.Size().Y, newBuffer.Data());
	return newBuffer;
}

void Graphics::SwapClipRect(Rect<int> &rect)
{
	std::swap(clipRect, rect);
	clipRect &= video.Size().OriginRect();
}

void Graphics::RenderZoom()
{
	if(!zoomEnabled)
		return;
	{
		int x, y, i, j;
		pixel pix;

		DrawFilledRect(RectSized(zoomWindowPosition, { zoomScopeSize * ZFACTOR, zoomScopeSize * ZFACTOR }), 0x000000_rgb);
		DrawRect(RectSized(zoomWindowPosition - Vec2{ 2, 2 }, Vec2{ zoomScopeSize*ZFACTOR+3, zoomScopeSize*ZFACTOR+3 }), 0xC0C0C0_rgb);
		DrawRect(RectSized(zoomWindowPosition - Vec2{ 1, 1 }, Vec2{ zoomScopeSize*ZFACTOR+1, zoomScopeSize*ZFACTOR+1 }), 0x000000_rgb);
		for (j=0; j<zoomScopeSize; j++)
			for (i=0; i<zoomScopeSize; i++)
			{
				pix = video[{ i + zoomScopePosition.X, j + zoomScopePosition.Y }];
				for (y=0; y<ZFACTOR-1; y++)
					for (x=0; x<ZFACTOR-1; x++)
						video[{ i * ZFACTOR + x + zoomWindowPosition.X, j * ZFACTOR + y + zoomWindowPosition.Y }] = pix;
			}
		if (zoomEnabled)
		{
			for (j=-1; j<=zoomScopeSize; j++)
			{
				XorPixel(zoomScopePosition + Vec2{ j, -1 });
				XorPixel(zoomScopePosition + Vec2{ j, zoomScopeSize });
			}
			for (j=0; j<zoomScopeSize; j++)
			{
				XorPixel(zoomScopePosition + Vec2{ -1, j });
				XorPixel(zoomScopePosition + Vec2{ zoomScopeSize, j });
			}
		}
	}
}
