#ifndef IMAGEREQUEST2_H
#define IMAGEREQUEST2_H

#include "Download.h"
#include "common/String.h"

#include <memory>

class VideoBuffer;
namespace http
{
	class ImageRequest : public Download
	{
		int Width, Height;
		
	public:
		ImageRequest(ByteString url, int width, int height);

		std::unique_ptr<VideoBuffer> Finish();
	};
}

#endif // IMAGEREQUEST2_H

