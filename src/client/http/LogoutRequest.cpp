#include "LogoutRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	LogoutRequest::LogoutRequest() :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Logout.json"), authRequire, false)
	{
		auto user = Client::Ref().GetAuthUser();
		AddPostData(FormData{
			{ "Key", user.SessionKey },
		});
	}

	void LogoutRequest::Finish()
	{
		APIRequest::Finish();
	}
}
