#ifndef IMAGEREQUEST2_H
#define IMAGEREQUEST2_H

#include "Request.h"
#include "common/String.h"

#include <memory>

class VideoBuffer;
namespace http
{
	class ImageRequest : public Request
	{
		int Width, Height;
		
	public:
		ImageRequest(ByteString url, int width, int height);
		virtual ~ImageRequest();

		std::unique_ptr<VideoBuffer> Finish();
	};
}

#endif // IMAGEREQUEST2_H

