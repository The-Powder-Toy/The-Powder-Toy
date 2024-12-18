#include "ReportSaveRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	ReportSaveRequest::ReportSaveRequest(int saveID, String message) :
		APIRequest(ByteString::Build(SERVER, "/Browse/Report.json?ID=", saveID, "&Key=", Client::Ref().GetAuthUser().SessionKey), authRequire, true)
	{
		AddPostData(FormData{
			{ "Reason", message.ToUtf8() },
		});
	}

	void ReportSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}
