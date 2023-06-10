#pragma once
#include "Request.h"

class SaveInfo;

namespace http
{
	class UploadSaveRequest : public Request
	{
	public:
		UploadSaveRequest(const SaveInfo &saveInfo);

		int Finish();
	};
}
