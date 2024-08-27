#include "SaveUserInfoRequest.h"
#include "Config.h"

namespace http
{
	SaveUserInfoRequest::SaveUserInfoRequest(UserInfo info) :
		APIRequest(ByteString::Build(SERVER, "/Profile.json"), authRequire, true)
	{
		AddPostData(FormData{
			{ "Location", info.location.ToUtf8() },
			{ "Biography", info.biography.ToUtf8() },
		});
	}

	void SaveUserInfoRequest::Finish()
	{
		APIRequest::Finish();
	}
}

