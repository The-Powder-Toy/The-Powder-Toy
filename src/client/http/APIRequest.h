#ifndef APIREQUEST2_H
#define APIREQUEST2_H

#include "Request.h"
#include "common/String.h"
#include "json/json.h"

#include <memory>
#include <map>

namespace http
{
	class APIRequest : public Request
	{
	public:
		struct Result
		{
			int status;
			std::unique_ptr<Json::Value> document;
		};

		APIRequest(ByteString url);
		virtual ~APIRequest();

		Result Finish();
	};
}

#endif // APIREQUEST2_H

