#pragma once
#include "Request.h"
#include <memory>

class SaveInfo;

namespace http
{
	class GetSaveRequest : public Request
	{
	public:
		GetSaveRequest(int saveID, int saveDate);

		std::unique_ptr<SaveInfo> Finish();
	};
}
