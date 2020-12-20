#include "APIRequest.h"

#include "client/Client.h"

namespace http
{
	APIRequest::APIRequest(ByteString url) : Request(url)
	{
		User user = Client::Ref().GetAuthUser();
		AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
	}

	APIRequest::~APIRequest()
	{
	}

	APIRequest::Result APIRequest::Finish()
	{
		Result result;
		try
		{
			ByteString data = Request::Finish(&result.status);
			// Note that at this point it's not safe to use any member of the
			// APIRequest object as Request::Finish signals RequestManager
			// to delete it.
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

