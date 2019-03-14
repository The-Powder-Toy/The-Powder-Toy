#include "SaveUserInfoRequest.h"

namespace http
{
	SaveUserInfoRequest::SaveUserInfoRequest(UserInfo info) :
		APIRequest(SCHEME SERVER "/Profile.json")
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
		// Note that at this point it's not safe to use any member of the
		// SaveUserInfoRequest object as Request::Finish signals RequestManager
		// to delete it.
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

