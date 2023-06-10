#pragma once
#include "APIRequest.h"

namespace http
{
	class PublishSaveRequest : public APIRequest
	{
	public:
		PublishSaveRequest(int saveID);

		void Finish();
	};
}
