#pragma once
#include "APIRequest.h"

class UserInfo;

namespace http
{
	class SaveUserInfoRequest : public APIRequest
	{
	public:
		SaveUserInfoRequest(UserInfo &info);

		bool Finish();
	};
}
