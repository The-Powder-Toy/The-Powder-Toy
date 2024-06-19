#include "UploadSaveRequest.h"
#include "client/SaveInfo.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "Config.h"

namespace http
{
	UploadSaveRequest::UploadSaveRequest(const SaveInfo &saveInfo) : Request(ByteString::Build(SERVER, "/Save.api"))
	{
		auto [ fromNewerVersion, gameData ] = saveInfo.GetGameSave()->Serialise();
		if (!gameData.size())
		{
			FailEarly("Cannot serialize game save");
			return;
		}
		else if (ALLOW_FAKE_NEWER_VERSION && fromNewerVersion && saveInfo.GetPublished())
		{
			FailEarly("Cannot publish save, incompatible with latest release version");
			return;
		}
		auto user = Client::Ref().GetAuthUser();
		if (!user.UserID)
		{
			FailEarly("Not authenticated");
			return;
		}
		AuthHeaders(ByteString::Build(user.UserID), user.SessionID);
		AddPostData(FormData{
			{ "Name", saveInfo.GetName().ToUtf8() },
			{ "Description", saveInfo.GetDescription().ToUtf8() },
			{ "Data", ByteString(gameData.begin(), gameData.end()), "save.bin" },
			{ "Publish", saveInfo.GetPublished() ? "Public" : "Private" },
			{ "Key", user.SessionKey },
		});
	}

	int UploadSaveRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseOk);
		int saveID = ByteString(data.begin() + 3, data.end()).ToNumber<int>();
		if (!saveID)
		{
			throw RequestError("Server did not return Save ID");
		}
		return saveID;
	}
}
