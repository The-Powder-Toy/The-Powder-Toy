#pragma once
#include "APIRequest.h"

namespace http
{
	class ExecVoteRequest : public APIRequest
	{
		int direction;

	public:
		ExecVoteRequest(int saveID, int newDirection);

		void Finish();

		int Direction() const
		{
			return direction;
		}
	};
}
