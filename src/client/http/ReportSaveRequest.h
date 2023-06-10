#pragma once
#include "APIRequest.h"

namespace http
{
	class ReportSaveRequest : public APIRequest
	{
	public:
		ReportSaveRequest(int saveID, String message);

		void Finish();
	};
}
