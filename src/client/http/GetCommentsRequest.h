#pragma once
#include "APIRequest.h"
#include "client/Comment.h"

namespace http
{
	class GetCommentsRequest : public APIRequest
	{
	public:
		GetCommentsRequest(int saveID, int start, int count);

		std::vector<Comment> Finish();
	};
}
