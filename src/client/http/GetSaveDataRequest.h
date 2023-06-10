#pragma once
#include "Request.h"

namespace http
{
	class GetSaveDataRequest : public Request
	{
	public:
		GetSaveDataRequest(int saveID, int saveDate);

		std::vector<char> Finish();
	};
}
