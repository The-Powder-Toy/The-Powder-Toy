#include "SaveUserInfoRequest.h"
#include "client/UserInfo.h"
#include "Config.h"

namespace http
{
	SaveUserInfoRequest::SaveUserInfoRequest(UserInfo &info) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Profile.json"))
	{
		AddPostData(FormData{
			{ "Location", info.location.ToUtf8() },
			{ "Biography", info.biography.ToUtf8() }
		});
	}

	bool SaveUserInfoRequest::Finish()
	{
		auto result = APIRequest::Finish();
		if (result.document)
		{
			return (*result.document)["Status"].asInt() == 1;
		}
		else
		{
			return false;
		}
	}
}

