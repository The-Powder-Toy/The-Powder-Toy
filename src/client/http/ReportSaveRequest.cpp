#include "ReportSaveRequest.h"
#include "Config.h"

namespace http
{
	ReportSaveRequest::ReportSaveRequest(int saveID, String message) :
		APIRequest({ ByteString::Build(SERVER, "/Browse/Report.json"), {
			{ "ID", ByteString::Build(saveID) },
		} }, authRequireAppendSession, true)
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
