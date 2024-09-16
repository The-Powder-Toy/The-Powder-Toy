#pragma once
#include "APIRequest.h"
#include <list>

namespace http
{
	class RemoveTagRequest : public APIRequest
	{
	public:
		RemoveTagRequest(int saveID, const ByteString& tag);

		std::list<ByteString> Finish();
	};
}
