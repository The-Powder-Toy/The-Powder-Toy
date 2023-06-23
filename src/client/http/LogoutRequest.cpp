#include "LogoutRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	LogoutRequest::LogoutRequest() :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Logout.json?Key=" + Client::Ref().GetAuthUser().SessionKey), authRequire, false)
	{
	}

	void LogoutRequest::Finish()
	{
		APIRequest::Finish();
	}
}
