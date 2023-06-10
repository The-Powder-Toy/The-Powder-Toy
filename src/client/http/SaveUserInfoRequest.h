#pragma once
#include "APIRequest.h"
#include "client/UserInfo.h"

namespace http
{
	class SaveUserInfoRequest : public APIRequest
	{
	public:
		SaveUserInfoRequest(UserInfo info);

		void Finish();
	};
}
