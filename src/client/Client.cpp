#include "Client.h"
#include "prefs/GlobalPrefs.h"
#include "client/http/Request.h"
#include "ClientListener.h"
#include "Format.h"
#include "MD5.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "client/UserInfo.h"
#include "common/platform/Platform.h"
#include "common/String.h"
#include "graphics/Graphics.h"
#include "prefs/Prefs.h"
#include "lua/CommandInterface.h"
#include "gui/preview/Comment.h"
#include "Config.h"
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <set>

Client::Client():
	messageOfTheDay("Fetching the message of the day..."),
	versionCheckRequest(nullptr),
	alternateVersionCheckRequest(nullptr),
	usingAltUpdateServer(false),
	updateAvailable(false),
	authUser(0, "")
{
	auto &prefs = GlobalPrefs::Ref();
	authUser.UserID = prefs.Get("User.ID", 0);
	authUser.Username = prefs.Get("User.Username", ByteString(""));
	authUser.SessionID = prefs.Get("User.SessionID", ByteString(""));
	authUser.SessionKey = prefs.Get("User.SessionKey", ByteString(""));
	auto elevation = prefs.Get("User.Elevation", ByteString(""));
	authUser.UserElevation = User::ElevationNone;
	if (elevation == "Admin")
	{
		authUser.UserElevation = User::ElevationAdmin;
	}
	if (elevation == "Mod")
	{
		authUser.UserElevation = User::ElevationModerator;
	}
	firstRun = !prefs.BackedByFile();
}

void Client::MigrateStampsDef()
{
	std::vector<char> data;
	if (!Platform::ReadFile(data, ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, "stamps.def")))
	{
		return;
	}
	for (auto i = 0; i < int(data.size()); i += 10)
	{
		stampIDs.push_back(ByteString(&data[0] + i, &data[0] + i + 10));
	}
}

void Client::Initialize()
{
	auto &prefs = GlobalPrefs::Ref();
	if (prefs.Get("version.update", false))
	{
		prefs.Set("version.update", false);
		Platform::UpdateFinish();
	}

	stamps = std::make_unique<Prefs>(ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, "stamps.json"));
	stampIDs = stamps->Get("MostRecentlyUsedFirst", std::vector<ByteString>{});
	{
		Prefs::DeferWrite dw(*stamps);
		if (!stamps->BackedByFile())
		{
			MigrateStampsDef();
			WriteStamps();
		}
		RescanStamps();
	}

	//Begin version check
	versionCheckRequest = std::make_unique<http::Request>(ByteString::Build(SCHEME, SERVER, "/Startup.json"));

	if (authUser.UserID)
	{
		versionCheckRequest->AuthHeaders(ByteString::Build(authUser.UserID), authUser.SessionID);
	}
	versionCheckRequest->Start();

	if constexpr (USE_UPDATESERVER)
	{
		// use an alternate update server
		alternateVersionCheckRequest = std::make_unique<http::Request>(ByteString::Build(SCHEME, UPDATESERVER, "/Startup.json"));
		usingAltUpdateServer = true;
		if (authUser.UserID)
		{
			alternateVersionCheckRequest->AuthHeaders(authUser.Username, "");
		}
		alternateVersionCheckRequest->Start();
	}
}

bool Client::IsFirstRun()
{
	return firstRun;
}

void Client::SetMessageOfTheDay(String message)
{
	messageOfTheDay = message;
	notifyMessageOfTheDay();
}

String Client::GetMessageOfTheDay()
{
	return messageOfTheDay;
}

void Client::AddServerNotification(std::pair<String, ByteString> notification)
{
	serverNotifications.push_back(notification);
	notifyNewNotification(notification);
}

std::vector<std::pair<String, ByteString> > Client::GetServerNotifications()
{
	return serverNotifications;
}

RequestStatus Client::ParseServerReturn(ByteString &result, int status, bool json)
{
	lastError = "";
	// no server response, return "Malformed Response"
	if (status == 200 && !result.size())
	{
		status = 603;
	}
	if (status == 302)
		return RequestOkay;
	if (status != 200)
	{
		lastError = String::Build("HTTP Error ", status, ": ", http::StatusText(status));
		return RequestFailure;
	}

	if (json)
	{
		std::istringstream datastream(result);
		Json::Value root;

		try
		{
			datastream >> root;
			// assume everything is fine if an empty [] is returned
			if (root.size() == 0)
			{
				return RequestOkay;
			}
			int status = root.get("Status", 1).asInt();
			if (status != 1)
			{
				lastError = ByteString(root.get("Error", "Unspecified Error").asString()).FromUtf8();
				return RequestFailure;
			}
		}
		catch (std::exception &e)
		{
			// sometimes the server returns a 200 with the text "Error: 401"
			if (!strncmp(result.c_str(), "Error: ", 7))
			{
				status = ByteString(result.begin() + 7, result.end()).ToNumber<int>();
				lastError = String::Build("HTTP Error ", status, ": ", http::StatusText(status));
				return RequestFailure;
			}
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
			return RequestFailure;
		}
	}
	else
	{
		if (strncmp(result.c_str(), "OK", 2))
		{
			lastError = result.FromUtf8();
			return RequestFailure;
		}
	}
	return RequestOkay;
}

void Client::Tick()
{
	CheckUpdate(versionCheckRequest, true);
	CheckUpdate(alternateVersionCheckRequest, false);
}

void Client::CheckUpdate(std::unique_ptr<http::Request> &updateRequest, bool checkSession)
{
	//Check status on version check request
	if (updateRequest && updateRequest->CheckDone())
	{
		auto [ status, data ] = updateRequest->Finish();

		if (checkSession && status == 618)
		{
			AddServerNotification({ "Failed to load SSL certificates", ByteString(SCHEME) + "powdertoy.co.uk/FAQ.html" });
		}

		if (status != 200)
		{
			//free(data);
			if (usingAltUpdateServer && !checkSession)
				this->messageOfTheDay = String::Build("HTTP Error ", status, " while checking for updates: ", http::StatusText(status));
			else
				this->messageOfTheDay = String::Build("HTTP Error ", status, " while fetching MotD");
		}
		else if(data.size())
		{
			std::istringstream dataStream(data);

			try
			{
				Json::Value objDocument;
				dataStream >> objDocument;

				//Check session
				if (checkSession)
				{
					if (!objDocument["Session"].asBool())
					{
						SetAuthUser(User(0, ""));
					}
				}

				//Notifications from server
				Json::Value notificationsArray = objDocument["Notifications"];
				for (Json::UInt j = 0; j < notificationsArray.size(); j++)
				{
					ByteString notificationLink = notificationsArray[j]["Link"].asString();
					String notificationText = ByteString(notificationsArray[j]["Text"].asString()).FromUtf8();

					std::pair<String, ByteString> item = std::pair<String, ByteString>(notificationText, notificationLink);
					AddServerNotification(item);
				}


				//MOTD
				if (!usingAltUpdateServer || !checkSession)
				{
					this->messageOfTheDay = ByteString(objDocument["MessageOfTheDay"].asString()).FromUtf8();
					notifyMessageOfTheDay();

					if constexpr (!IGNORE_UPDATES)
					{
						//Check for updates
						Json::Value versions = objDocument["Updates"];
						if constexpr (!SNAPSHOT)
						{
							Json::Value stableVersion = versions["Stable"];
							int stableMajor = stableVersion["Major"].asInt();
							int stableMinor = stableVersion["Minor"].asInt();
							int stableBuild = stableVersion["Build"].asInt();
							ByteString stableFile = stableVersion["File"].asString();
							String stableChangelog = ByteString(stableVersion["Changelog"].asString()).FromUtf8();
							if (stableBuild > BUILD_NUM)
							{
								updateAvailable = true;
								updateInfo = UpdateInfo(stableMajor, stableMinor, stableBuild, stableFile, stableChangelog, UpdateInfo::Stable);
							}
						}

						if (!updateAvailable)
						{
							Json::Value betaVersion = versions["Beta"];
							int betaMajor = betaVersion["Major"].asInt();
							int betaMinor = betaVersion["Minor"].asInt();
							int betaBuild = betaVersion["Build"].asInt();
							ByteString betaFile = betaVersion["File"].asString();
							String betaChangelog = ByteString(betaVersion["Changelog"].asString()).FromUtf8();
							if (betaBuild > BUILD_NUM)
							{
								updateAvailable = true;
								updateInfo = UpdateInfo(betaMajor, betaMinor, betaBuild, betaFile, betaChangelog, UpdateInfo::Beta);
							}
						}

						if constexpr (SNAPSHOT || MOD)
						{
							Json::Value snapshotVersion = versions["Snapshot"];
							int snapshotSnapshot = snapshotVersion["Snapshot"].asInt();
							ByteString snapshotFile = snapshotVersion["File"].asString();
							String snapshotChangelog = ByteString(snapshotVersion["Changelog"].asString()).FromUtf8();
							if (snapshotSnapshot > SNAPSHOT_ID)
							{
								updateAvailable = true;
								updateInfo = UpdateInfo(snapshotSnapshot, snapshotFile, snapshotChangelog, UpdateInfo::Snapshot);
							}
						}

						if(updateAvailable)
						{
							notifyUpdateAvailable();
						}
					}
				}
			}
			catch (std::exception & e)
			{
				//Do nothing
			}
		}
		updateRequest.reset();
	}
}

UpdateInfo Client::GetUpdateInfo()
{
	return updateInfo;
}

void Client::notifyUpdateAvailable()
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyUpdateAvailable(this);
	}
}

void Client::notifyMessageOfTheDay()
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyMessageOfTheDay(this);
	}
}

void Client::notifyAuthUserChanged()
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyAuthUserChanged(this);
	}
}

void Client::notifyNewNotification(std::pair<String, ByteString> notification)
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyNewNotification(this, notification);
	}
}

void Client::AddListener(ClientListener * listener)
{
	listeners.push_back(listener);
}

void Client::RemoveListener(ClientListener * listener)
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		if((*iterator) == listener)
		{
			listeners.erase(iterator);
			return;
		}
	}
}

Client::~Client()
{
}

void Client::SetAuthUser(User user)
{
	authUser = user;
	{
		auto &prefs = GlobalPrefs::Ref();
		Prefs::DeferWrite dw(prefs);
		if (authUser.UserID)
		{
			prefs.Set("User.ID", authUser.UserID);
			prefs.Set("User.SessionID", authUser.SessionID);
			prefs.Set("User.SessionKey", authUser.SessionKey);
			prefs.Set("User.Username", authUser.Username);
			ByteString elevation = "None";
			if (authUser.UserElevation == User::ElevationAdmin)
			{
				elevation = "Admin";
			}
			if (authUser.UserElevation == User::ElevationModerator)
			{
				elevation = "Mod";
			}
			prefs.Set("User.Elevation", elevation);
		}
		else
		{
			prefs.Clear("User");
		}
	}
	notifyAuthUserChanged();
}

User Client::GetAuthUser()
{
	return authUser;
}

RequestStatus Client::UploadSave(SaveInfo & save)
{
	lastError = "";
	int dataStatus;
	ByteString data;
	ByteString userID = ByteString::Build(authUser.UserID);
	if (authUser.UserID)
	{
		if (!save.GetGameSave())
		{
			lastError = "Empty game save";
			return RequestFailure;
		}

		save.SetID(0);

		auto [ fromNewerVersion, gameData ] = save.GetGameSave()->Serialise();
		(void)fromNewerVersion;

		if (!gameData.size())
		{
			lastError = "Cannot serialize game save";
			return RequestFailure;
		}
		else if (ALLOW_FAKE_NEWER_VERSION && fromNewerVersion && save.GetPublished())
		{
			lastError = "Cannot publish save, incompatible with latest release version.";
			return RequestFailure;
		}

		std::tie(dataStatus, data) = http::Request::SimpleAuth(ByteString::Build(SCHEME, SERVER, "/Save.api"), userID, authUser.SessionID, {
			{ "Name", save.GetName().ToUtf8() },
			{ "Description", save.GetDescription().ToUtf8() },
			{ "Data:save.bin", ByteString(gameData.begin(), gameData.end()) },
			{ "Publish", save.GetPublished() ? "Public" : "Private" },
			{ "Key", authUser.SessionKey }
		});
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}

	RequestStatus ret = ParseServerReturn(data, dataStatus, false);
	if (ret == RequestOkay)
	{
		int saveID = ByteString(data.begin() + 3, data.end()).ToNumber<int>();
		if (!saveID)
		{
			lastError = "Server did not return Save ID";
			ret = RequestFailure;
		}
		else
			save.SetID(saveID);
	}
	return ret;
}

void Client::MoveStampToFront(ByteString stampID)
{
	auto it = std::find(stampIDs.begin(), stampIDs.end(), stampID);
	auto changed = false;
	if (it == stampIDs.end())
	{
		stampIDs.push_back(stampID);
		it = stampIDs.end() - 1;
		changed = true;
	}
	else if (it != stampIDs.begin())
	{
		changed = true;
	}
	if (changed)
	{
		std::rotate(stampIDs.begin(), it, it + 1);
		WriteStamps();
	}
}

std::unique_ptr<SaveFile> Client::GetStamp(ByteString stampID)
{
	ByteString stampFile = ByteString(ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, stampID, ".stm"));
	auto saveFile = LoadSaveFile(stampFile);
	if (!saveFile)
		saveFile = LoadSaveFile(stampID);
	else
		saveFile->SetDisplayName(stampID.FromUtf8());
	return saveFile;
}

void Client::DeleteStamp(ByteString stampID)
{
	auto it = std::remove(stampIDs.begin(), stampIDs.end(), stampID);
	if (it != stampIDs.end())
	{
		stampIDs.erase(it, stampIDs.end());
		Platform::RemoveFile(ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, stampID, ".stm"));
		WriteStamps();
	}
}

ByteString Client::AddStamp(std::unique_ptr<GameSave> saveData)
{
	auto now = (uint64_t)time(NULL);
	if (lastStampTime != now)
	{
		lastStampTime = now;
		lastStampName = 0;
	}
	else
	{
		lastStampName += 1;
	}
	ByteString saveID, filename;
	while (true)
	{
		saveID = ByteString::Build(Format::Hex(Format::Width(lastStampTime, 8)), Format::Hex(Format::Width(lastStampName, 2)));
		filename = ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, saveID, ".stm");
		if (!Platform::FileExists(filename))
		{
			break;
		}
		lastStampName += 1;
	}

	Platform::MakeDirectory(STAMPS_DIR);

	Json::Value stampInfo;
	stampInfo["type"] = "stamp";
	stampInfo["username"] = authUser.Username;
	stampInfo["name"] = filename;
	stampInfo["date"] = Json::Value::UInt64(now);
	if (authors.size() != 0)
	{
		// This is a stamp, always append full authorship info (even if same user)
		stampInfo["links"].append(Client::Ref().authors);
	}
	saveData->authors = stampInfo;

	auto [ fromNewerVersion, gameData ] = saveData->Serialise();
	(void)fromNewerVersion;
	if (!gameData.size())
		return "";

	Platform::WriteFile(gameData, filename);
	MoveStampToFront(saveID);
	return saveID;
}

void Client::RescanStamps()
{
	ByteString extension = ".stm";
	std::set<ByteString> stampFilesSet;
	for (auto &stampID : Platform::DirectorySearch("stamps", "", { extension }))
	{
		stampFilesSet.insert(stampID.substr(0, stampID.size() - extension.size()));
	}
	std::vector<ByteString> newStampIDs;
	auto changed = false;
	for (auto &stampID : stampIDs)
	{
		if (stampFilesSet.find(stampID) == stampFilesSet.end())
		{
			changed = true;
		}
		else
		{
			newStampIDs.push_back(stampID);
		}
	}
	auto oldCount = newStampIDs.size();
	auto stampIDsSet = std::set<ByteString>(stampIDs.begin(), stampIDs.end());
	for (auto &stampID : stampFilesSet)
	{
		if (stampIDsSet.find(stampID) == stampIDsSet.end())
		{
			newStampIDs.push_back(stampID);
			changed = true;
		}
	}
	if (changed)
	{
		// Move newly discovered stamps to front.
		std::rotate(newStampIDs.begin(), newStampIDs.begin() + oldCount, newStampIDs.end());
		stampIDs = newStampIDs;
		WriteStamps();
	}
}

void Client::WriteStamps()
{
	if (stampIDs.size())
	{
		stamps->Set("MostRecentlyUsedFirst", stampIDs);
	}
}

const std::vector<ByteString> &Client::GetStamps() const
{
	return stampIDs;
}

RequestStatus Client::ExecVote(int saveID, int direction)
{
	lastError = "";
	int dataStatus;
	ByteString data;

	if (authUser.UserID)
	{
		ByteString saveIDText = ByteString::Build(saveID);
		ByteString userIDText = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(ByteString::Build(SCHEME, SERVER, "/Vote.api"), userIDText, authUser.SessionID, {
			{ "ID", saveIDText },
			{ "Action", direction ? (direction == 1 ? "Up" : "Down") : "Reset" },
			{ "Key", authUser.SessionKey }
		});
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, false);
	return ret;
}

std::vector<char> Client::GetSaveData(int saveID, int saveDate)
{
	lastError = "";
	ByteString urlStr;
	if (saveDate)
		urlStr = ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, "_", saveDate, ".cps");
	else
		urlStr = ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, ".cps");

	auto [ dataStatus, data ] = http::Request::Simple(urlStr);

	// will always return failure
	ParseServerReturn(data, dataStatus, false);
	if (data.size() && dataStatus == 200)
	{
		return std::vector<char>(data.begin(), data.end());
	}
	return {};
}

LoginStatus Client::Login(ByteString username, ByteString password, User & user)
{
	lastError = "";

	user.UserID = 0;
	user.Username = "";
	user.SessionID = "";
	user.SessionKey = "";

	auto [ dataStatus, data ] = http::Request::Simple(ByteString::Build("https://", SERVER, "/Login.json"), {
		{ "name", username },
		{ "pass", password },
	});

	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	if (ret == RequestOkay)
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

			ByteString usernameTemp = objDocument["Username"].asString();
			int userIDTemp = objDocument["UserID"].asInt();
			ByteString sessionIDTemp = objDocument["SessionID"].asString();
			ByteString sessionKeyTemp = objDocument["SessionKey"].asString();
			ByteString userElevationTemp = objDocument["Elevation"].asString();

			Json::Value notificationsArray = objDocument["Notifications"];
			for (Json::UInt j = 0; j < notificationsArray.size(); j++)
			{
				ByteString notificationLink = notificationsArray[j]["Link"].asString();
				String notificationText = ByteString(notificationsArray[j]["Text"].asString()).FromUtf8();

				std::pair<String, ByteString> item = std::pair<String, ByteString>(notificationText, notificationLink);
				AddServerNotification(item);
			}

			user.Username = usernameTemp;
			user.UserID = userIDTemp;
			user.SessionID = sessionIDTemp;
			user.SessionKey = sessionKeyTemp;
			ByteString userElevation = userElevationTemp;
			if(userElevation == "Admin")
				user.UserElevation = User::ElevationAdmin;
			else if(userElevation == "Mod")
				user.UserElevation = User::ElevationModerator;
			else
				user.UserElevation= User::ElevationNone;
			return LoginOkay;
		}
		catch (std::exception &e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
			return LoginError;
		}
	}
	return LoginError;
}

RequestStatus Client::DeleteSave(int saveID)
{
	lastError = "";
	ByteString data;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/Delete.json?ID=", saveID, "&Mode=Delete&Key=", authUser.SessionKey);
	int dataStatus;
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	return ret;
}

RequestStatus Client::AddComment(int saveID, String comment)
{
	lastError = "";
	ByteString data;
	int dataStatus;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/Comments.json?ID=", saveID);
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID, {
			{ "Comment", comment.ToUtf8() },
			{ "Key", authUser.SessionKey }
		});
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	return ret;
}

RequestStatus Client::FavouriteSave(int saveID, bool favourite)
{
	lastError = "";
	ByteStringBuilder urlStream;
	ByteString data;
	int dataStatus;
	urlStream << SCHEME << SERVER << "/Browse/Favourite.json?ID=" << saveID << "&Key=" << authUser.SessionKey;
	if(!favourite)
		urlStream << "&Mode=Remove";
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(urlStream.Build(), userID, authUser.SessionID);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	return ret;
}

RequestStatus Client::ReportSave(int saveID, String message)
{
	lastError = "";
	ByteString data;
	int dataStatus;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/Report.json?ID=", saveID, "&Key=", authUser.SessionKey);
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID, {
			{ "Reason", message.ToUtf8() },
		});
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	return ret;
}

RequestStatus Client::UnpublishSave(int saveID)
{
	lastError = "";
	ByteString data;
	int dataStatus;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/Delete.json?ID=", saveID, "&Mode=Unpublish&Key=", authUser.SessionKey);
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	return ret;
}

RequestStatus Client::PublishSave(int saveID)
{
	lastError = "";
	ByteString data;
	int dataStatus;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/View.json?ID=", saveID, "&Key=", authUser.SessionKey);
	if (authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID, {
			{ "ActionPublish", "bagels" },
		});
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	return ret;
}

std::unique_ptr<SaveInfo> Client::GetSave(int saveID, int saveDate)
{
	lastError = "";
	ByteStringBuilder urlStream;
	urlStream << SCHEME << SERVER  << "/Browse/View.json?ID=" << saveID;
	if(saveDate)
	{
		urlStream << "&Date=" << saveDate;
	}
	ByteString data;
	int dataStatus;
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(urlStream.Build(), userID, authUser.SessionID);
	}
	else
	{
		std::tie(dataStatus, data) = http::Request::Simple(urlStream.Build());
	}
	if(dataStatus == 200 && data.size())
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

			int tempID = objDocument["ID"].asInt();
			int tempScoreUp = objDocument["ScoreUp"].asInt();
			int tempScoreDown = objDocument["ScoreDown"].asInt();
			int tempMyScore = objDocument["ScoreMine"].asInt();
			ByteString tempUsername = objDocument["Username"].asString();
			String tempName = ByteString(objDocument["Name"].asString()).FromUtf8();
			String tempDescription = ByteString(objDocument["Description"].asString()).FromUtf8();
			int tempCreatedDate = objDocument["DateCreated"].asInt();
			int tempUpdatedDate = objDocument["Date"].asInt();
			bool tempPublished = objDocument["Published"].asBool();
			bool tempFavourite = objDocument["Favourite"].asBool();
			int tempComments = objDocument["Comments"].asInt();
			int tempViews = objDocument["Views"].asInt();
			int tempVersion = objDocument["Version"].asInt();

			Json::Value tagsArray = objDocument["Tags"];
			std::list<ByteString> tempTags;
			for (Json::UInt j = 0; j < tagsArray.size(); j++)
				tempTags.push_back(tagsArray[j].asString());

			auto tempSave = std::make_unique<SaveInfo>(tempID, tempCreatedDate, tempUpdatedDate, tempScoreUp,
			                                   tempScoreDown, tempMyScore, tempUsername, tempName,
			                                   tempDescription, tempPublished, tempTags);
			tempSave->Comments = tempComments;
			tempSave->Favourite = tempFavourite;
			tempSave->Views = tempViews;
			tempSave->Version = tempVersion;
			return tempSave;
		}
		catch (std::exception & e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
			return nullptr;
		}
	}
	else
	{
		lastError = http::StatusText(dataStatus);
	}
	return nullptr;
}

std::unique_ptr<SaveFile> Client::LoadSaveFile(ByteString filename)
{
	ByteString err;
	std::unique_ptr<SaveFile> file;
	if (Platform::FileExists(filename))
	{
		file = std::make_unique<SaveFile>(filename);
		try
		{
			std::vector<char> data;
			if (Platform::ReadFile(data, filename))
			{
				file->SetGameSave(std::make_unique<GameSave>(std::move(data)));
			}
			else
			{
				err = "failed to open";
			}
		}
		catch (const ParseException &e)
		{
			err = e.what();
		}
	}
	else
	{
		err = "does not exist";
	}
	if (err.size())
	{
		std::cerr << "Client: " << filename << ": " << err << std::endl;
		if (file)
		{
			file->SetLoadingError(err.FromUtf8());
		}
		commandInterface->SetLastError(err.FromUtf8());
	}
	return file;
}

std::list<ByteString> * Client::RemoveTag(int saveID, ByteString tag)
{
	lastError = "";
	std::list<ByteString> * tags = NULL;
	ByteString data;
	int dataStatus;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/EditTag.json?Op=delete&ID=", saveID, "&Tag=", tag, "&Key=", authUser.SessionKey);
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID);
	}
	else
	{
		lastError = "Not authenticated";
		return NULL;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	if (ret == RequestOkay)
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value responseObject;
			dataStream >> responseObject;

			Json::Value tagsArray = responseObject["Tags"];
			tags = new std::list<ByteString>();
			for (Json::UInt j = 0; j < tagsArray.size(); j++)
				tags->push_back(tagsArray[j].asString());
		}
		catch (std::exception &e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
		}
	}
	return tags;
}

std::list<ByteString> * Client::AddTag(int saveID, ByteString tag)
{
	lastError = "";
	std::list<ByteString> * tags = NULL;
	ByteString data;
	int dataStatus;
	ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/EditTag.json?Op=add&ID=", saveID, "&Tag=", tag, "&Key=", authUser.SessionKey);
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		std::tie(dataStatus, data) = http::Request::SimpleAuth(url, userID, authUser.SessionID);
	}
	else
	{
		lastError = "Not authenticated";
		return NULL;
	}
	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	if (ret == RequestOkay)
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value responseObject;
			dataStream >> responseObject;

			Json::Value tagsArray = responseObject["Tags"];
			tags = new std::list<ByteString>();
			for (Json::UInt j = 0; j < tagsArray.size(); j++)
				tags->push_back(tagsArray[j].asString());
		}
		catch (std::exception & e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
		}
	}
	return tags;
}

// stamp-specific wrapper for MergeAuthorInfo
// also used for clipboard and lua stamps
void Client::MergeStampAuthorInfo(Json::Value stampAuthors)
{
	if (stampAuthors.size())
	{
		// when loading stamp/clipboard, only append info to authorship info (since we aren't replacing the save)
		// unless there is nothing loaded currently, then set authors directly
		if (authors.size())
		{
			// Don't add if it's exactly the same
			if (stampAuthors["links"].size() == 1 && stampAuthors["links"][0] == Client::Ref().authors)
				return;
			if (authors["username"] != stampAuthors["username"])
			{
				// 2nd arg of MergeAuthorInfo needs to be an array
				Json::Value toAdd;
				toAdd.append(stampAuthors);
				MergeAuthorInfo(toAdd);
			}
			else if (stampAuthors["links"].size())
			{
				MergeAuthorInfo(stampAuthors["links"]);
			}
		}
		else
			authors = stampAuthors;
	}
}

// linksToAdd is an array (NOT an object) of links to add to authors["links"]
void Client::MergeAuthorInfo(Json::Value linksToAdd)
{
	for (Json::Value::ArrayIndex i = 0; i < linksToAdd.size(); i++)
	{
		// link is the same exact json we have open, don't do anything
		if (linksToAdd[i] == authors)
			return;

		bool hasLink = false;
		for (Json::Value::ArrayIndex j = 0; j < authors["links"].size(); j++)
		{
			// check everything in authors["links"] to see if it's the same json as what we are already adding
			if (authors["links"][j] == linksToAdd[i])
				hasLink = true;
		}
		if (!hasLink)
			authors["links"].append(linksToAdd[i]);
	}
}

// load current authors information into a json value (when saving everything: stamps, clipboard, local saves, and online saves)
void Client::SaveAuthorInfo(Json::Value *saveInto)
{
	if (authors.size() != 0)
	{
		// Different username? Save full original save info
		if (authors["username"] != (*saveInto)["username"])
			(*saveInto)["links"].append(authors);
		// This is probalby the same save
		// Don't append another layer of links, just keep existing links
		else if (authors["links"].size())
			(*saveInto)["links"] = authors["links"];
	}
}

bool AddCustomGol(String ruleString, String nameString, unsigned int highColor, unsigned int lowColor)
{
	auto &prefs = GlobalPrefs::Ref();
	auto customGOLTypes = prefs.Get("CustomGOL.Types", std::vector<ByteString>{});
	std::vector<ByteString> newCustomGOLTypes;
	bool nameTaken = false;
	for (auto gol : customGOLTypes)
	{
		auto parts = gol.FromUtf8().PartitionBy(' ');
		if (parts.size())
		{
			if (parts[0] == nameString)
			{
				nameTaken = true;
			}
		}
		newCustomGOLTypes.push_back(gol);
	}
	if (nameTaken)
		return false;

	StringBuilder sb;
	sb << nameString << " " << ruleString << " " << highColor << " " << lowColor;
	newCustomGOLTypes.push_back(sb.Build().ToUtf8());
	prefs.Set("CustomGOL.Types", newCustomGOLTypes);
	return true;
}

String Client::DoMigration(ByteString fromDir, ByteString toDir)
{
	if (fromDir.at(fromDir.length() - 1) != '/')
		fromDir = fromDir + '/';
	if (toDir.at(toDir.length() - 1) != '/')
		toDir = toDir + '/';

	std::ofstream logFile(fromDir + "/migrationlog.txt", std::ios::out);
	logFile << "Running migration of data from " << fromDir + " to " << toDir << std::endl;

	// Get lists of files to migrate
	auto stamps = Platform::DirectorySearch(fromDir + "stamps", "", { ".stm" });
	auto saves = Platform::DirectorySearch(fromDir + "Saves", "", { ".cps", ".stm" });
	auto scripts = Platform::DirectorySearch(fromDir + "scripts", "", { ".lua", ".txt" });
	auto downloadedScripts = Platform::DirectorySearch(fromDir + "scripts/downloaded", "", { ".lua" });
	bool hasScriptinfo = Platform::FileExists(toDir + "scripts/downloaded/scriptinfo");
	auto screenshots = Platform::DirectorySearch(fromDir, "screenshot", { ".png" });
	bool hasAutorun = Platform::FileExists(fromDir + "autorun.lua");
	bool hasPref = Platform::FileExists(fromDir + "powder.pref");

	if (stamps.empty() && saves.empty() && scripts.empty() && downloadedScripts.empty() && screenshots.empty() && !hasAutorun && !hasPref)
	{
		logFile << "Nothing to migrate.";
		return "Nothing to migrate. This button is used to migrate data from pre-96.0 TPT installations to the shared directory";
	}

	StringBuilder result;
	std::stack<ByteString> dirsToDelete;

	// Migrate a list of files
	auto migrateList = [&](std::vector<ByteString> list, ByteString directory, String niceName) {
		result << '\n' << niceName << ": ";
		if (!list.empty() && !directory.empty())
			Platform::MakeDirectory(toDir + directory);
		int migratedCount = 0, failedCount = 0;
		for (auto &item : list)
		{
			std::string from = fromDir + directory + "/" + item;
			std::string to = toDir + directory + "/" + item;
			if (!Platform::FileExists(to))
			{
				if (Platform::RenameFile(from, to, false))
				{
					failedCount++;
					logFile << "failed to move " << from << " to " << to << std::endl;
				}
				else
				{
					migratedCount++;
					logFile << "moved " << from << " to " << to << std::endl;
				}
			}
			else
			{
				logFile << "skipping " << from << "(already exists)" << std::endl;
			}
		}

		dirsToDelete.push(directory);
		result << "\bt" << migratedCount << " migratated\x0E, \br" << failedCount << " failed\x0E";
		int duplicates = list.size() - migratedCount - failedCount;
		if (duplicates)
			result << ", " << list.size() - migratedCount - failedCount << " skipped (duplicate)";
	};

	// Migrate a single file
	auto migrateFile = [&fromDir, &toDir, &result, &logFile](ByteString filename) {
		ByteString from = fromDir + filename;
		ByteString to = toDir + filename;
		if (!Platform::FileExists(to))
		{
			if (Platform::RenameFile(from, to, false))
			{
				logFile << "failed to move " << from << " to " << to << std::endl;
				result << "\n\br" << filename.FromUtf8() << " migration failed\x0E";
			}
			else
			{
				logFile << "moved " << from << " to " << to << std::endl;
				result << '\n' << filename.FromUtf8() << " migrated";
			}
		}
		else
		{
			logFile << "skipping " << from << "(already exists)" << std::endl;
			result << '\n' << filename.FromUtf8() << " skipped (already exists)";
		}

		if (!Platform::RemoveFile(fromDir + filename)) {
			logFile << "failed to delete " << filename << std::endl;
		}
	};

	// Do actual migration
	Platform::RemoveFile(fromDir + "stamps/stamps.def");
	Platform::RemoveFile(fromDir + "stamps/stamps.json");
	migrateList(stamps, "stamps", "Stamps");
	migrateList(saves, "Saves", "Saves");
	if (!scripts.empty())
		migrateList(scripts, "scripts", "Scripts");
	if (!hasScriptinfo && !downloadedScripts.empty())
	{
		migrateList(downloadedScripts, "scripts/downloaded", "Downloaded scripts");
		migrateFile("scripts/downloaded/scriptinfo");
	}
	if (!screenshots.empty())
		migrateList(screenshots, "", "Screenshots");
	if (hasAutorun)
		migrateFile("autorun.lua");
	if (hasPref)
		migrateFile("powder.pref");

	// Delete leftover directories
	while (!dirsToDelete.empty())
	{
		ByteString toDelete = dirsToDelete.top();
		if (!Platform::DeleteDirectory(fromDir + toDelete)) {
			logFile << "failed to delete " << toDelete << std::endl;
		}
		dirsToDelete.pop();
	}

	// chdir into the new directory
	Platform::ChangeDir(toDir);

	RescanStamps();

	logFile << std::endl << std::endl << "Migration complete. Results: " << result.Build().ToUtf8();
	logFile.close();

	return result.Build();
}
