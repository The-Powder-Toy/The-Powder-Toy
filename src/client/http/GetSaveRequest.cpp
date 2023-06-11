#include "GetSaveRequest.h"
#include "client/Client.h"
#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "Config.h"

namespace http
{
	static ByteString Url(int saveID, int saveDate)
	{
		ByteStringBuilder builder;
		builder << SCHEME << SERVER  << "/Browse/View.json?ID=" << saveID;
		if (saveDate)
		{
			builder << "&Date=" << saveDate;
		}
		return builder.Build();
	}

	GetSaveRequest::GetSaveRequest(int saveID, int saveDate) : Request(Url(saveID, saveDate))
	{
		auto user = Client::Ref().GetAuthUser();
		if (user.UserID)
		{
			// This is needed so we know how we rated this save.
			AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
		}
	}

	std::unique_ptr<SaveInfo> GetSaveRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseData);
		std::unique_ptr<SaveInfo> saveInfo;
		try
		{
			Json::Value document;
			std::istringstream ss(data);
			ss >> document;
			std::list<ByteString> tags;
			for (auto &tag : document["Tags"])
			{
				tags.push_back(tag.asString());
			}
			saveInfo = std::make_unique<SaveInfo>(
				document["ID"].asInt(),
				document["DateCreated"].asInt64(),
				document["Date"].asInt64(),
				document["ScoreUp"].asInt(),
				document["ScoreDown"].asInt(),
				document["ScoreMine"].asInt(),
				document["Username"].asString(),
				ByteString(document["Name"].asString()).FromUtf8(),
				ByteString(document["Description"].asString()).FromUtf8(),
				document["Published"].asBool(),
				tags
			);
			saveInfo->Comments = document["Comments"].asInt();
			saveInfo->Favourite = document["Favourite"].asBool();
			saveInfo->Views = document["Views"].asInt();
			saveInfo->Version = document["Version"].asInt();
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return saveInfo;
	}
}
