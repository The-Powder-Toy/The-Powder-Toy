#include "ImageRequest.h"
#include "graphics/Graphics.h"
#include <iostream>

namespace http
{
	ImageRequest::ImageRequest(ByteString url, Vec2<int> size):
		Request(std::move(url)),
		size(size)
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
			vb = VideoBuffer::FromPNG(std::vector<char>(data.begin(), data.end()));
			if (vb)
				vb->Resize(size, true);
			else
			{
				vb = std::make_unique<VideoBuffer>(Vec2(15, 16));
				vb->BlendChar(Vec2(2, 4), 0xE06E, 0xFFFFFF_rgb .WithAlpha(0xFF));
			}
		}
		return vb;
	}
}

