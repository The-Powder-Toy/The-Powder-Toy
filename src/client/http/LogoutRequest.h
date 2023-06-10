#pragma once
#include "APIRequest.h"

namespace http
{
	class LogoutRequest : public APIRequest
	{
	public:
		LogoutRequest();

		void Finish();
	};
}
