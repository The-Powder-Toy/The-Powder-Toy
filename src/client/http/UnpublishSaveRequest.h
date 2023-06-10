#pragma once
#include "APIRequest.h"

namespace http
{
	class UnpublishSaveRequest : public APIRequest
	{
	public:
		UnpublishSaveRequest(int saveID);

		void Finish();
	};
}
