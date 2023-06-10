#pragma once
#include "APIRequest.h"
#include "client/UserInfo.h"

namespace http
{
	class GetUserInfoRequest : public APIRequest
	{
	public:
		GetUserInfoRequest(ByteString username);

		UserInfo Finish();
	};
}
