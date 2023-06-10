#include "LoginRequest.h"
#include "Config.h"
#include "client/Client.h"
#include <json/json.h>

namespace http
{
	LoginRequest::LoginRequest(ByteString username, ByteString password) : Request(ByteString::Build("https://", SERVER, "/Login.json"))
	{
		AddPostData(FormData{
			{ "name", username },
			{ "pass", password },
		});
	}

	LoginInfo LoginRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseJson);
		LoginInfo loginInfo = { { 0, "" }, {} };
		try
		{
			Json::Value document;
			std::istringstream ss(data);
			ss >> document;
			loginInfo.user.Username = document["Username"].asString();
			loginInfo.user.UserID = document["UserID"].asInt();
			loginInfo.user.SessionID = document["SessionID"].asString();
			loginInfo.user.SessionKey = document["SessionKey"].asString();
			loginInfo.user.UserElevation = User::ElevationFromString(document["Elevation"].asString());
			for (auto &item : document["Notifications"])
			{
				loginInfo.notifications.push_back({
					ByteString(item["Text"].asString()).FromUtf8(),
					item["Link"].asString(),
				});
			}
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return loginInfo;
	}
}
