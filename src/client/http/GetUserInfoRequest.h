#pragma once
#include "APIRequest.h"

class UserInfo;

namespace http
{
	class GetUserInfoRequest : public APIRequest
	{
	public:
		GetUserInfoRequest(ByteString username);

		std::unique_ptr<UserInfo> Finish();
	};
}
