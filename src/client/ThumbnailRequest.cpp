#include "ThumbnailRequest.h"
#include "Config.h"

namespace http
{
	ThumbnailRequest::ThumbnailRequest(int saveID, int saveDate, int width, int height) :
		ImageRequest((
			saveDate
			? ByteString::Build("http://" STATICSERVER "/", saveID, "_", saveDate, "_small.pti")
			: ByteString::Build("http://" STATICSERVER "/", saveID, "_small.pti")
		), width, height)
	{
	}
}

