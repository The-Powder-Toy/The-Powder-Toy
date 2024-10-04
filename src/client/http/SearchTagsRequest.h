#pragma once
#include "APIRequest.h"

namespace http
{
	class SearchTagsRequest : public APIRequest
	{
	public:
		SearchTagsRequest(int start, int count, const ByteString& query);

		std::vector<std::pair<ByteString, int>> Finish();
	};
}
