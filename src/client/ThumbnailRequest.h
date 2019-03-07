#ifndef THUMBNAILREQUEST2_H
#define THUMBNAILREQUEST2_H

#include "ImageRequest.h"
#include "common/String.h"

namespace http
{
	class ThumbnailRequest : public ImageRequest
	{
	public:
		ThumbnailRequest(int saveID, int saveDate, int width, int height);
		virtual ~ThumbnailRequest();
	};
}

#endif // THUMBNAILREQUEST2_H

