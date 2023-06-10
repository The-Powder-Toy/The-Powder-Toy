#include "UnpublishSaveRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	UnpublishSaveRequest::UnpublishSaveRequest(int saveID) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Browse/Delete.json?ID=", saveID, "&Mode=Unpublish&Key=", Client::Ref().GetAuthUser().SessionKey), authRequire, true)
	{
	}

	void UnpublishSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}

