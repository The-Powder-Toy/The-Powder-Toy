#include "GetUserInfoRequest.h"
#include "client/UserInfo.h"
#include "Config.h"

namespace http
{
	GetUserInfoRequest::GetUserInfoRequest(ByteString username) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/User.json?Name=", username), authOmit, false)
	{
	}

	UserInfo GetUserInfoRequest::Finish()
	{
		auto result = APIRequest::Finish();
		UserInfo userInfo;
		try
		{
			auto &user = result["User"];
			userInfo = UserInfo(
				user["ID"].asInt(),
				user["Age"].asInt(),
				user["Username"].asString(),
				ByteString(user["Biography"].asString()).FromUtf8(),
				ByteString(user["Location"].asString()).FromUtf8(),
				user["Website"].asString(),
				user["Saves"]["Count"].asInt(),
				user["Saves"]["AverageScore"].asFloat(),
				user["Saves"]["HighestScore"].asInt(),
				user["Forum"]["Topics"].asInt(),
				user["Forum"]["Replies"].asInt(),
				user["Forum"]["Reputation"].asInt()
			);
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return userInfo;
	}
}

