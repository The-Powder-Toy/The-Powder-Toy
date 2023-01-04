#pragma once
#include "ImageRequest.h"

namespace http
{
	class ThumbnailRequest : public ImageRequest
	{
	public:
		ThumbnailRequest(int saveID, int saveDate, int width, int height);
		virtual ~ThumbnailRequest();
	};
}
