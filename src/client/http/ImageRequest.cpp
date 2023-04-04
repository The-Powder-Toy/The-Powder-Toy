#include "ImageRequest.h"
#include "graphics/Graphics.h"
#include <iostream>

namespace http
{
	ImageRequest::ImageRequest(ByteString url, Vec2<int> size):
		Request(std::move(url)),
		size(size)
	{}

	ImageRequest::ImageRequest(ByteString url, int width, int height):
		ImageRequest(url, Vec2(width, height))
	{}

	ImageRequest::~ImageRequest()
	{}

	std::unique_ptr<VideoBuffer> ImageRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		(void)status; // We don't use this for anything, not ideal >_>
		std::unique_ptr<VideoBuffer> vb;
		if (data.size())
		{
			int imgw, imgh;
			std::vector<pixel> imageData;
			if (PngDataToPixels(imageData, imgw, imgh, data.data(), data.size(), true))
			{
				vb = std::make_unique<VideoBuffer>(imageData.data(), Vec2(imgw, imgh));
			}
			else
			{
				vb = std::make_unique<VideoBuffer>(Vec2(32, 32));
				vb->BlendChar(Vec2(14, 14), 'x', 0xFFFFFF_rgb .WithAlpha(0xFF));
			}
			vb->Resize(size, true);
		}
		return vb;
	}
}

