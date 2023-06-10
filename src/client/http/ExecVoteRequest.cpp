#include "ExecVoteRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	ExecVoteRequest::ExecVoteRequest(int saveID, int newDirection) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Vote.api"), authRequire, false),
		direction(newDirection)
	{
		auto user = Client::Ref().GetAuthUser();
		if (!user.UserID)
		{
			FailEarly("Not authenticated");
			return;
		}
		AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
		AddPostData(FormData{
			{ "ID", ByteString::Build(saveID) },
			{ "Action", direction ? (direction == 1 ? "Up" : "Down") : "Reset" },
			{ "Key", user.SessionKey },
		});
	}

	void ExecVoteRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseOk);
	}
}
