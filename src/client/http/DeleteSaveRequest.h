#pragma once
#include "APIRequest.h"

namespace http
{
	class DeleteSaveRequest : public APIRequest
	{
	public:
		DeleteSaveRequest(int saveID);

		void Finish();
	};
}
