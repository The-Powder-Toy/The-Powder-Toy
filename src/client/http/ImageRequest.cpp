#include "ImageRequest.h"

#include "common/Singleton.h"
#include "graphics/Graphics.h"
#include "Config.h"

#include <iostream>

namespace http
{
	ImageRequest::ImageRequest(ByteString url, int width, int height) :
		Request(url),
		Width(width),
		Height(height)
	{
	}

	ImageRequest::~ImageRequest()
	{
	}

	std::unique_ptr<VideoBuffer> ImageRequest::Finish()
	{
		int width = Width;
		int height = Height;
		ByteString data = Request::Finish(nullptr);
		// Note that at this point it's not safe to use any member of the
		// ImageRequest object as Request::Finish signals RequestManager
		// to delete it.
		std::unique_ptr<VideoBuffer> vb;
		if (data.size())
		{
			int imgw, imgh;
			std::vector<pixel> imageData;
			if (PngDataToPixels(imageData, imgw, imgh, data.data(), data.size(), true))
			{
				vb = std::unique_ptr<VideoBuffer>(new VideoBuffer(imageData.data(), imgw, imgh));
			}
			else
			{
				vb = std::unique_ptr<VideoBuffer>(new VideoBuffer(32, 32));
				vb->SetCharacter(14, 14, 'x', 255, 255, 255, 255);
			}
			vb->Resize(width, height, true);
		}
		return vb;
	}
}

