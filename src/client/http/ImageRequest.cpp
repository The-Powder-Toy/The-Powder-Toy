#include "ImageRequest.h"
#include "graphics/Graphics.h"
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
		auto [ status, data ] = Request::Finish();
		(void)status; // We don't use this for anything, not ideal >_>
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

