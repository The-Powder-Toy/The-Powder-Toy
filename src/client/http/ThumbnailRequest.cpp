#include "ThumbnailRequest.h"
#include "Config.h"

namespace http
{
	ThumbnailRequest::ThumbnailRequest(int saveID, int saveDate, int width, int height) :
		ImageRequest((
			saveDate
			? ByteString::Build(SCHEME STATICSERVER "/", saveID, "_", saveDate, "_small.pti")
			: ByteString::Build(SCHEME STATICSERVER "/", saveID, "_small.pti")
		), width, height)
	{
	}

	ThumbnailRequest::~ThumbnailRequest()
	{
	}
}

