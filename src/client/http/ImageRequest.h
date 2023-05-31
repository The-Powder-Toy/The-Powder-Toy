#pragma once
#include "common/String.h"
#include "common/Vec2.h"
#include "Request.h"

#include <memory>

class VideoBuffer;

namespace http
{
	class ImageRequest : public Request
	{
		Vec2<int> size;

	public:
		ImageRequest(ByteString url, Vec2<int> size);

		std::unique_ptr<VideoBuffer> Finish();
	};
}
