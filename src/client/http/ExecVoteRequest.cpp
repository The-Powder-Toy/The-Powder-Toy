#include "ExecVoteRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	ExecVoteRequest::ExecVoteRequest(int saveID, int newDirection) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Vote.api"), authRequire, false),
		direction(newDirection)
	{
		AddPostData(FormData{
			{ "ID", ByteString::Build(saveID) },
			{ "Action", direction ? (direction == 1 ? "Up" : "Down") : "Reset" },
			{ "Key", Client::Ref().GetAuthUser().SessionKey },
		});
	}

	void ExecVoteRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseOk);
	}
}
