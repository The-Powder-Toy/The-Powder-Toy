#include "AvatarRequest.h"
#include "Config.h"

namespace http
{
	AvatarRequest::AvatarRequest(ByteString username, int width, int height) :
		ImageRequest(ByteString::Build("http://" STATICSERVER "/avatars/", username, ".pti"), width, height)
	{
	}

	AvatarRequest::~AvatarRequest()
	{
	}
}

