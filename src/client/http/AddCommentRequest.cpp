#include "AddCommentRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	AddCommentRequest::AddCommentRequest(int saveID, String comment) :
		APIRequest({ ByteString::Build(SERVER, "/Browse/Comments.json"), {
			{ "ID", ByteString::Build(saveID) },
		} }, authRequire, true)
	{
		auto user = Client::Ref().GetAuthUser();
		AddPostData(FormData{
			{ "Comment", comment.ToUtf8() },
			{ "Key", user ? user->SessionKey : ByteString("") },
		});
	}

	void AddCommentRequest::Finish()
	{
		APIRequest::Finish();
	}
}
