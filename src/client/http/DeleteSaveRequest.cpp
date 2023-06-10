#include "DeleteSaveRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	DeleteSaveRequest::DeleteSaveRequest(int saveID) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Browse/Delete.json?ID=", saveID, "&Mode=Delete&Key=", Client::Ref().GetAuthUser().SessionKey), authRequire, true)
	{
	}

	void DeleteSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}
