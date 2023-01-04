#pragma once
#include "APIRequest.h"

class UserInfo;

namespace http
{
	class GetUserInfoRequest : public APIRequest
	{
	public:
		GetUserInfoRequest(ByteString username);
		virtual ~GetUserInfoRequest();

		std::unique_ptr<UserInfo> Finish();
	};
}
