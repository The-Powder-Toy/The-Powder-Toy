#include "StartupRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	// TODO: update Client::messageOfTheDay
	StartupRequest::StartupRequest(bool newAlternate) :
		Request(ByteString::Build(SCHEME, newAlternate ? UPDATESERVER : SERVER, "/Startup.json")),
		alternate(newAlternate)
	{
		auto user = Client::Ref().GetAuthUser();
		if (user.UserID)
		{
			if (alternate)
			{
				// Cursed
				AuthHeaders(user.Username, "");
			}
			else
			{
				AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
			}
		}
	}

	StartupInfo StartupRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseJson);
		StartupInfo startupInfo;
		try
		{
			Json::Value document;
			std::istringstream ss(data);
			ss >> document;
			startupInfo.sessionGood = document["Session"].asBool();
			startupInfo.messageOfTheDay = ByteString(document["MessageOfTheDay"].asString()).FromUtf8();
			for (auto &notification : document["Notifications"])
			{
				startupInfo.notifications.push_back({
					ByteString(notification["Text"].asString()).FromUtf8(),
					notification["Link"].asString()
				});
			}
			if constexpr (!IGNORE_UPDATES)
			{
				auto &versions = document["Updates"];
				auto parseUpdate = [this, &versions, &startupInfo](ByteString key, UpdateInfo::Channel channel, std::function<bool (int)> updateAvailableFunc) {
					if (!versions.isMember(key))
					{
						return;
					}
					auto &info = versions[key];
					auto getOr = [&info](ByteString key, int defaultValue) -> int {
						if (!info.isMember(key))
						{
							return defaultValue;
						}
						return info[key].asInt();
					};
					auto build = getOr(key == "Snapshot" ? "Snapshot" : "Build", -1);
					if (!updateAvailableFunc(build))
					{
						return;
					}
					startupInfo.updateInfo = UpdateInfo{
						channel,
						ByteString::Build(SCHEME, alternate ? UPDATESERVER : SERVER, info["File"].asString()),
						ByteString(info["Changelog"].asString()).FromUtf8(),
						getOr("Major", -1),
						getOr("Minor", -1),
						build,
					};
				};
				if constexpr (SNAPSHOT || MOD)
				{
					parseUpdate("Snapshot", UpdateInfo::channelSnapshot, [](int build) -> bool {
						return build > SNAPSHOT_ID;
					});
				}
				else
				{
					parseUpdate("Stable", UpdateInfo::channelStable, [](int build) -> bool {
						return build > BUILD_NUM;
					});
					if (!startupInfo.updateInfo.has_value())
					{
						parseUpdate("Beta", UpdateInfo::channelBeta, [](int build) -> bool {
							return build > BUILD_NUM;
						});
					}
				}
			}
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return startupInfo;
	}
}
