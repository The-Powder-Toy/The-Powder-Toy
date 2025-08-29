#pragma once
#include "Request.h"
#include "common/String.h"
#include "Format.h"
#include <json/json.h>

namespace http
{
	class APIRequest : public Request
	{
		bool checkStatus;

	public:
		enum AuthMode
		{
			authRequire,
			authRequireAppendSession,
			authUse,
			authOmit,
		};
		APIRequest(format::Url url, AuthMode authMode, bool newCheckStatus);

		Json::Value Finish();
	};
}
