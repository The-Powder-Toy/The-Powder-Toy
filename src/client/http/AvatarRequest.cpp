#include "AvatarRequest.h"
#include "Config.h"

namespace http
{
	AvatarRequest::AvatarRequest(ByteString username, int width, int height) :
		ImageRequest(ByteString::Build(SCHEME STATICSERVER "/avatars/", username, ".pti"), width, height)
	{
	}

	AvatarRequest::~AvatarRequest()
	{
	}
}

