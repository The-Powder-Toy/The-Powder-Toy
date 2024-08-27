#include "PublishSaveRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	PublishSaveRequest::PublishSaveRequest(int saveID) :
		APIRequest(ByteString::Build(SERVER, "/Browse/View.json?ID=", saveID, "&Key=", Client::Ref().GetAuthUser().SessionKey), authRequire, true)
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
