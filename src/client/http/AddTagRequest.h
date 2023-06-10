#pragma once
#include "APIRequest.h"
#include <list>

namespace http
{
	class AddTagRequest : public APIRequest
	{
	public:
		AddTagRequest(int saveID, ByteString tag);

		std::list<ByteString> Finish();
	};
}
