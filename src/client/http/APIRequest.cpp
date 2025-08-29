#include "APIRequest.h"
#include "client/Client.h"

namespace http
{
	namespace
	{
		format::Url &MaybeAppendSession(APIRequest::AuthMode authMode, format::Url &url)
		{
			if (auto user = Client::Ref().GetAuthUser(); user && authMode == APIRequest::authRequireAppendSession)
			{
				url.params["Key"] = user->SessionKey;
			}
			return url;
		}
	}

	APIRequest::APIRequest(format::Url url, AuthMode authMode, bool newCheckStatus) : Request(MaybeAppendSession(authMode, url).ToByteString()), checkStatus(newCheckStatus)
	{
		auto user = Client::Ref().GetAuthUser();
		if ((authMode == authRequire ||
		     authMode == authRequireAppendSession) && !user)
		{
			FailEarly("Not authenticated");
			return;
		}
		if (authMode != authOmit && user)
		{
			AuthHeaders(ByteString::Build(user->UserID), user->SessionID);
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
