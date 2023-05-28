#include "ThumbnailRequest.h"
#include "Config.h"

namespace http
{
	ThumbnailRequest::ThumbnailRequest(int saveID, int saveDate, Vec2<int> size) :
		ImageRequest((
			saveDate
			? ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, "_", saveDate, "_small.png")
			: ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, "_small.png")
		), size)
	{
	}
}

