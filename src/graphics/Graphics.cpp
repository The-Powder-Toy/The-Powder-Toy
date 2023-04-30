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

std::unique_ptr<VideoBuffer> VideoBuffer::FromPNG(std::vector<char> const &data)
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

template class RasterDrawMethods<VideoBuffer>;

Graphics::Graphics()
{}

void Graphics::draw_icon(int x, int y, Icon icon, unsigned char alpha, bool invert)
{
	y--;
	switch(icon)
	{
	case IconOpen:
		if(invert)
			BlendChar({ x, y }, 0xE001, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 0xE001, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconReload:
		if(invert)
			BlendChar({ x, y }, 0xE011, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 0xE011, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconSave:
		if(invert)
			BlendChar({ x, y }, 0xE002, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 0xE002, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconVoteUp:
		if(invert)
		{
			BlendChar({ x-11, y+1 }, 0xE04B, RGBA<uint8_t>(0, 100, 0, alpha));
			BlendText({ x+2, y+1 }, "Vote", RGBA<uint8_t>(0, 100, 0, alpha));
		}
		else
		{
			BlendChar({ x-11, y+1 }, 0xE04B, RGBA<uint8_t>(0, 187, 18, alpha));
			BlendText({ x+2, y+1 }, "Vote", RGBA<uint8_t>(0, 187, 18, alpha));
		}
		break;
	case IconVoteDown:
		if(invert)
			BlendChar({ x, y }, 0xE04A, RGBA<uint8_t>(100, 10, 0, alpha));
		else
			BlendChar({ x, y }, 0xE04A, RGBA<uint8_t>(187, 40, 0, alpha));
		break;
	case IconTag:
		if(invert)
			BlendChar({ x, y }, 0xE003, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 0xE003, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconNew:
		if(invert)
			BlendChar({ x, y }, 0xE012, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 0xE012, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconLogin:
		if(invert)
			BlendChar({ x, y + 1 }, 0xE004, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y + 1 }, 0xE004, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconSimulationSettings:
		if(invert)
			BlendChar({ x, y + 1 }, 0xE04F, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y + 1 }, 0xE04F, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconRenderSettings:
		if(invert)
		{
			BlendChar({ x, y + 1 }, 0xE058, RGBA<uint8_t>(255, 0, 0, alpha));
			BlendChar({ x, y + 1 }, 0xE059, RGBA<uint8_t>(0, 255, 0, alpha));
			BlendChar({ x, y + 1 }, 0xE05A, RGBA<uint8_t>(0, 0, 255, alpha));
		}
		else
		{
			AddChar({ x, y + 1 }, 0xE058, RGBA<uint8_t>(255, 0, 0, alpha));
			AddChar({ x, y + 1 }, 0xE059, RGBA<uint8_t>(0, 255, 0, alpha));
			AddChar({ x, y + 1 }, 0xE05A, RGBA<uint8_t>(0, 0, 255, alpha));
		}
		break;
	case IconPause:
		if(invert)
			BlendChar({ x, y }, 0xE010, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 0xE010, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconFavourite:
		if(invert)
			BlendChar({ x, y }, 0xE04C, RGBA<uint8_t>(100, 80, 32, alpha));
		else
			BlendChar({ x, y }, 0xE04C, RGBA<uint8_t>(192, 160, 64, alpha));
		break;
	case IconReport:
		if(invert)
			BlendChar({ x, y }, 0xE063, RGBA<uint8_t>(140, 140, 0, alpha));
		else
			BlendChar({ x, y }, 0xE063, RGBA<uint8_t>(255, 255, 0, alpha));
		break;
	case IconUsername:
		if(invert)
		{
			BlendChar({ x, y }, 0xE00B, RGBA<uint8_t>(32, 64, 128, alpha));
			BlendChar({ x, y }, 0xE00A, RGBA<uint8_t>(0, 0, 0, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE00B, RGBA<uint8_t>(32, 64, 128, alpha));
			BlendChar({ x, y }, 0xE00A, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconPassword:
		if(invert)
		{
			BlendChar({ x, y }, 0xE00C, RGBA<uint8_t>(160, 144, 32, alpha));
			BlendChar({ x, y }, 0xE004, RGBA<uint8_t>(0, 0, 0, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE00C, RGBA<uint8_t>(160, 144, 32, alpha));
			BlendChar({ x, y }, 0xE004, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconClose:
		if(invert)
			BlendChar({ x, y }, 0xE02A, RGBA<uint8_t>(20, 20, 20, alpha));
		else
			BlendChar({ x, y }, 0xE02A, RGBA<uint8_t>(230, 230, 230, alpha));
		break;
	case IconVoteSort:
		if (invert)
		{
			BlendChar({ x, y }, 0xE029, RGBA<uint8_t>(44, 48, 32, alpha));
			BlendChar({ x, y }, 0xE028, RGBA<uint8_t>(32, 44, 32, alpha));
			BlendChar({ x, y }, 0xE027, RGBA<uint8_t>(128, 128, 128, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE029, RGBA<uint8_t>(144, 48, 32, alpha));
			BlendChar({ x, y }, 0xE028, RGBA<uint8_t>(32, 144, 32, alpha));
			BlendChar({ x, y }, 0xE027, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconDateSort:
		if (invert)
		{
			BlendChar({ x, y }, 0xE026, RGBA<uint8_t>(32, 32, 32, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE026, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconMyOwn:
		if (invert)
		{
			BlendChar({ x, y }, 0xE014, RGBA<uint8_t>(192, 160, 64, alpha));
			BlendChar({ x, y }, 0xE013, RGBA<uint8_t>(32, 32, 32, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE014, RGBA<uint8_t>(192, 160, 64, alpha));
			BlendChar({ x, y }, 0xE013, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconSearch:
		BlendChar({ x, y }, 0xE00E, RGBA<uint8_t>(30, 30, 180, alpha));
		BlendChar({ x, y }, 0xE00F, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconDelete:
		if(invert)
		{
			BlendChar({ x, y }, 0xE006, RGBA<uint8_t>(159, 47, 31, alpha));
			BlendChar({ x, y }, 0xE005, RGBA<uint8_t>(0, 0, 0, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE006, RGBA<uint8_t>(159, 47, 31, alpha));
			BlendChar({ x, y }, 0xE005, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconAdd:
		if(invert)
		{
			BlendChar({ x, y }, 0xE006, RGBA<uint8_t>(32, 144, 32, alpha));
			BlendChar({ x, y }, 0xE009, RGBA<uint8_t>(0, 0, 0, alpha));
		}
		else
		{
			BlendChar({ x, y }, 0xE006, RGBA<uint8_t>(32, 144, 32, alpha));
			BlendChar({ x, y }, 0xE009, RGBA<uint8_t>(255, 255, 255, alpha));
		}
		break;
	case IconVelocity:
		BlendChar({ x + 1, y }, 0xE018, RGBA<uint8_t>(128, 160, 255, alpha));
		break;
	case IconPressure:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE019, RGBA<uint8_t>(180, 160, 16, alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE019, RGBA<uint8_t>(255, 212, 32, alpha));
		break;
	case IconPersistant:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE01A, RGBA<uint8_t>(20, 20, 20, alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE01A, RGBA<uint8_t>(212, 212, 212, alpha));
		break;
	case IconFire:
		BlendChar({ x + 1, y + 1 }, 0xE01B, RGBA<uint8_t>(255, 0, 0, alpha));
		BlendChar({ x + 1, y + 1 }, 0xE01C, RGBA<uint8_t>(255, 255, 64, alpha));
		break;
	case IconBlob:
		if(invert)
			BlendChar({ x + 1, y }, 0xE03F, RGBA<uint8_t>(55, 180, 55, alpha));
		else
			BlendChar({ x + 1, y }, 0xE03F, RGBA<uint8_t>(55, 255, 55, alpha));
		break;
	case IconHeat:
		BlendChar({ x + 3, y }, 0xE03E, RGBA<uint8_t>(255, 0, 0, alpha));
		if(invert)
			BlendChar({ x + 3, y }, 0xE03D, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x + 3, y }, 0xE03D, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconBlur:
		if(invert)
			BlendChar({ x + 1, y }, 0xE044, RGBA<uint8_t>(50, 70, 180, alpha));
		else
			BlendChar({ x + 1, y }, 0xE044, RGBA<uint8_t>(100, 150, 255, alpha));
		break;
	case IconGradient:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE053, RGBA<uint8_t>(255, 50, 255, alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE053, RGBA<uint8_t>(205, 50, 205, alpha));
		break;
	case IconLife:
		if(invert)
			BlendChar({ x, y + 1 }, 0xE060, RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y + 1 }, 0xE060, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconEffect:
		BlendChar({ x + 1, y }, 0xE061, RGBA<uint8_t>(255, 255, 160, alpha));
		break;
	case IconGlow:
		BlendChar({ x + 1, y }, 0xE05F, RGBA<uint8_t>(200, 255, 255, alpha));
		break;
	case IconWarp:
		BlendChar({ x + 1, y }, 0xE05E, RGBA<uint8_t>(255, 255, 255, alpha));
		break;
	case IconBasic:
		if(invert)
			BlendChar({ x + 1, y + 1 }, 0xE05B, RGBA<uint8_t>(50, 50, 0, alpha));
		else
			BlendChar({ x + 1, y + 1 }, 0xE05B, RGBA<uint8_t>(255, 255, 200, alpha));
		break;
	case IconAltAir:
		if(invert) {
			BlendChar({ x + 1, y + 1 }, 0xE054, RGBA<uint8_t>(180, 55, 55, alpha));
			BlendChar({ x + 1, y + 1 }, 0xE055, RGBA<uint8_t>(55, 180, 55, alpha));
		} else {
			BlendChar({ x + 1, y + 1 }, 0xE054, RGBA<uint8_t>(255, 55, 55, alpha));
			BlendChar({ x + 1, y + 1 }, 0xE055, RGBA<uint8_t>(55, 255, 55, alpha));
		}
		break;
	default:
		if(invert)
			BlendChar({ x, y }, 't', RGBA<uint8_t>(0, 0, 0, alpha));
		else
			BlendChar({ x, y }, 't', RGBA<uint8_t>(255, 255, 255, alpha));
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

bool Graphics::GradientStop::operator <(const GradientStop &other) const
{
	return point < other.point;
}

std::vector<RGB<uint8_t>> Graphics::Gradient(std::vector<GradientStop> stops, int resolution)
{
	std::vector<RGB<uint8_t>> table(resolution, 0x000000_rgb);
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
			table[i] = left.color.Blend(right.color.WithAlpha(f * 0xFF));
		}
	}
	return table;
}
