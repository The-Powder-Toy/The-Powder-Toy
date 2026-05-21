#include "LogoutRequest.h"
#include "Config.h"

namespace http
{
	LogoutRequest::LogoutRequest() :
		APIRequest({ ByteString::Build(SERVER, "/Logout.json") }, authRequireAppendSession, true)
	{
	}

	void LogoutRequest::Finish()
	{
		APIRequest::Finish();
	}
}
