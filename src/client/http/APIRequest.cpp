#include "APIRequest.h"

#include "client/Client.h"

namespace http
{
	APIRequest::APIRequest(ByteString url) : Request(url)
	{
		User user = Client::Ref().GetAuthUser();
		AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
	}

	APIRequest::Result APIRequest::Finish()
	{
		Result result;
		try
		{
			ByteString data;
			std::tie(result.status, data) = Request::Finish();
			Client::Ref().ParseServerReturn(data, result.status, true);
			if (result.status == 200 && data.size())
			{
				std::istringstream dataStream(data);
				Json::Value objDocument;
				dataStream >> objDocument;
				result.document = std::unique_ptr<Json::Value>(new Json::Value(objDocument));
			}
		}
		catch (std::exception & e)
		{
		}
		return result;
	}
}

