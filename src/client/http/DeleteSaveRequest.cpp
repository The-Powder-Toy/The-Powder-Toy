#include "DeleteSaveRequest.h"
#include "Config.h"

namespace http
{
	DeleteSaveRequest::DeleteSaveRequest(int saveID) :
		APIRequest(ByteString::Build(SERVER, "/Browse/Delete.json?ID=", saveID, "&Mode=Delete"), authRequireAppendSession, true)
	{
	}

	void DeleteSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}
