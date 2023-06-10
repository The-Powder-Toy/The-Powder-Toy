#include "AddCommentRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	AddCommentRequest::AddCommentRequest(int saveID, String comment) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Browse/Comments.json?ID=", saveID), authRequire, true)
	{
		auto user = Client::Ref().GetAuthUser();
		AddPostData(FormData{
			{ "Comment", comment.ToUtf8() },
			{ "Key", user.SessionKey },
		});
	}

	void AddCommentRequest::Finish()
	{
		APIRequest::Finish();
	}
}
