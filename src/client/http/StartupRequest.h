#pragma once
#include "APIRequest.h"
#include "client/StartupInfo.h"

namespace http
{
	class StartupRequest : public Request
	{
		bool alternate;

	public:
		StartupRequest(bool newAlternate);

		StartupInfo Finish();
	};
}
