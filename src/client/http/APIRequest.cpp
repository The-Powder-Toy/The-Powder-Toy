#include "APIRequest.h"
#include "client/Client.h"

namespace http
{
	APIRequest::APIRequest(ByteString url, AuthMode authMode, bool newCheckStatus) : Request(url), checkStatus(newCheckStatus)
	{
		auto user = Client::Ref().GetAuthUser();
		if (authMode == authRequire && !user.UserID)
		{
			FailEarly("Not authenticated");
			return;
		}
		if (authMode != authOmit && user.UserID)
		{
			AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
		}
	}

	Json::Value APIRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, checkStatus ? responseJson : responseData);
		Json::Value document;
		try
		{
			std::istringstream ss(data);
			ss >> document;
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return document;
	}
}
