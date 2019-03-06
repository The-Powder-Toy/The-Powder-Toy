#ifndef AVATARREQUEST2_H
#define AVATARREQUEST2_H

#include "ImageRequest.h"
#include "common/String.h"

#include <memory>

namespace http
{
	class AvatarRequest : public ImageRequest
	{
	public:
		AvatarRequest(ByteString username, int width, int height);
	};
}

#endif // AVATARREQUEST2_H

