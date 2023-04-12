#pragma once
#include "ImageRequest.h"

namespace http
{
	class ThumbnailRequest : public ImageRequest
	{
	public:
		ThumbnailRequest(int saveID, int saveDate, Vec2<int> size);
		virtual ~ThumbnailRequest();
	};
}
