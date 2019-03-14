#include "ThumbnailRequest.h"
#include "Config.h"

namespace http
{
	ThumbnailRequest::ThumbnailRequest(int saveID, int saveDate, int width, int height) :
		ImageRequest((
			saveDate
			? ByteString::Build(STATICSCHEME STATICSERVER "/", saveID, "_", saveDate, "_small.pti")
			: ByteString::Build(STATICSCHEME STATICSERVER "/", saveID, "_small.pti")
		), width, height)
	{
	}

	ThumbnailRequest::~ThumbnailRequest()
	{
	}
}

