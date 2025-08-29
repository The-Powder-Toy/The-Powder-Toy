#include "PublishSaveRequest.h"
#include "Config.h"

namespace http
{
	PublishSaveRequest::PublishSaveRequest(int saveID) :
		APIRequest({ ByteString::Build(SERVER, "/Browse/View.json"), {
			{ "ID", ByteString::Build(saveID) },
		} }, authRequireAppendSession, true)
	{
		AddPostData(FormData{
			{ "ActionPublish", "bagels" },
		});
	}

	void PublishSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}
