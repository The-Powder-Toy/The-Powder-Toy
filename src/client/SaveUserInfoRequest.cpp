#include "SaveUserInfoRequest.h"

namespace http
{
	SaveUserInfoRequest::SaveUserInfoRequest(UserInfo info) :
		APIRequest("http://" SERVER "/Profile.json")
	{
		AddPostData({
			{ "Location", info.location.ToUtf8() },
			{ "Biography", info.biography.ToUtf8() }
		});
	}

	SaveUserInfoRequest::~SaveUserInfoRequest()
	{
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

