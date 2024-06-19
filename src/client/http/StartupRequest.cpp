#include "StartupRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	// TODO: update Client::messageOfTheDay
	StartupRequest::StartupRequest(bool newAlternate) :
		Request(ByteString::Build(newAlternate ? UPDATESERVER : SERVER, "/Startup.json")),
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
				auto parseUpdate = [this, &versions, &startupInfo](ByteString key, UpdateInfo::Channel channel) {
					if (!versions.isMember(key))
					{
						return;
					}
					auto &info = versions[key];
					if (info.isNull())
					{
						return;
					}
					auto getOr = [&info](ByteString key, int defaultValue) -> int {
						if (!info.isMember(key))
						{
							return defaultValue;
						}
						return info[key].asInt();
					};
					auto build = getOr(key == "Snapshot" ? "Snapshot" : "Build", 0);
					if (size_t(build) <= APP_VERSION.build)
					{
						return;
					}
					startupInfo.updateInfo = UpdateInfo{
						channel,
						ByteString::Build(alternate ? UPDATESERVER : SERVER, info["File"].asString()),
						ByteString(info["Changelog"].asString()).FromUtf8(),
						getOr("Major", 0),
						getOr("Minor", 0),
						build,
					};
				};
				if constexpr (SNAPSHOT || MOD)
				{
					parseUpdate("Snapshot", UpdateInfo::channelSnapshot);
				}
				else
				{
					parseUpdate("Stable", UpdateInfo::channelStable);
					if (!startupInfo.updateInfo.has_value())
					{
						parseUpdate("Beta", UpdateInfo::channelBeta);
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
