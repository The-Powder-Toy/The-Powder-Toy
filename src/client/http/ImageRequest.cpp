#include "ImageRequest.h"

#include "common/Singleton.h"
#include "graphics/Graphics.h"
#include "Config.h"

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
			vb->Resize(width, height, true);
		}
		return vb;
	}
}

