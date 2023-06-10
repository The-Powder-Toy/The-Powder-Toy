#pragma once
#include "APIRequest.h"
#include "client/SaveInfo.h"
#include "client/Search.h"

namespace http
{
	class SearchSavesRequest : public APIRequest
	{
	public:
		SearchSavesRequest(int start, int count, ByteString query, Sort sort, Category category);

		std::pair<int, std::vector<std::unique_ptr<SaveInfo>>> Finish();
	};
}
