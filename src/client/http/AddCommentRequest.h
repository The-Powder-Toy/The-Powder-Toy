#pragma once
#include "APIRequest.h"

namespace http
{
	class AddCommentRequest : public APIRequest
	{
	public:
		AddCommentRequest(int saveID, const String& comment);

		void Finish();
	};
}
