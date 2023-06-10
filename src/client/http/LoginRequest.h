#pragma once
#include "Request.h"
#include "client/LoginInfo.h"

namespace http
{
	class LoginRequest : public Request
	{
	public:
		LoginRequest(ByteString username, ByteString password);

		LoginInfo Finish();
	};
}
