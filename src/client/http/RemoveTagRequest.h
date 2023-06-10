#pragma once
#include "APIRequest.h"
#include <list>

namespace http
{
	class RemoveTagRequest : public APIRequest
	{
	public:
		RemoveTagRequest(int saveID, ByteString tag);

		std::list<ByteString> Finish();
	};
}
