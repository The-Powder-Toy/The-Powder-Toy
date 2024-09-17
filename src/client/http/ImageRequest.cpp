#include "ImageRequest.h"
#include "client/Client.h"
#include "graphics/VideoBuffer.h"

#include <iostream>
#include <utility>

namespace http
{
	ImageRequest::ImageRequest(ByteString url, Vec2<int> newRequestedSize) : Request(std::move(url)), requestedSize(newRequestedSize)
	{
	}

	std::unique_ptr<VideoBuffer> ImageRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseData);
		auto vb = VideoBuffer::FromPNG(std::vector<char>(data.begin(), data.end()));
		if (vb)
		{
			vb->Resize(requestedSize, true);
		}
		else
		{
			vb = std::make_unique<VideoBuffer>(Vec2(15, 16));
			vb->BlendChar(Vec2(2, 4), 0xE06E, 0xFFFFFF_rgb .WithAlpha(0xFF));
		}
		return vb;
	}
}

