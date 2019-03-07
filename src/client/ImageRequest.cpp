#include "ImageRequest.h"

#include "common/Singleton.h"
#include "graphics/Graphics.h"
#include "Config.h"

namespace http
{
	ImageRequest::ImageRequest(ByteString url, int width, int height) :
		Download(url),
		Width(width),
		Height(height)
	{
	}

	ImageRequest::~ImageRequest()
	{
	}

	std::unique_ptr<VideoBuffer> ImageRequest::Finish()
	{
		ByteString data = Download::Finish(nullptr);
		std::unique_ptr<VideoBuffer> vb;
		if (data.size())
		{
			int imgw, imgh;
			pixel *imageData = Graphics::ptif_unpack(&data[0], data.size(), &imgw, &imgh);
			if (imageData)
			{
				vb = std::unique_ptr<VideoBuffer>(new VideoBuffer(imageData, imgw, imgh));
				free(imageData);
			}
			else
			{
				vb = std::unique_ptr<VideoBuffer>(new VideoBuffer(32, 32));
				vb->SetCharacter(14, 14, 'x', 255, 255, 255, 255);
			}
			vb->Resize(Width, Height, true);
		}
		return vb;
	}
}

