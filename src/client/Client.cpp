#include "Client.h"

#include "client/http/Request.h" // includes curl.h, needs to come first to silence a warning on windows

#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <fstream>

#ifdef MACOSX
# include "common/macosx.h"
#endif

#ifdef LIN
# include "icon_cps.png.h"
# include "icon_exe.png.h"
# include "save.xml.h"
# include "powder.desktop.h"
#endif

#ifdef WIN
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <shlobj.h>
# include <objidl.h>
# include <shlwapi.h>
# include <windows.h>
# include <direct.h>
# include "resource.h"
#else
# include <sys/stat.h>
# include <unistd.h>
#endif

#include "ClientListener.h"
#include "Config.h"
#include "Format.h"
#include "MD5.h"
#include "Update.h"

#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "client/UserInfo.h"
#include "common/Platform.h"
#include "common/String.h"
#include "graphics/Graphics.h"

#ifdef LUACONSOLE
# include "lua/LuaScriptInterface.h"
#endif

#include "client/http/RequestManager.h"
#include "gui/preview/Comment.h"

Client::Client():
	messageOfTheDay("Fetching the message of the day..."),
	versionCheckRequest(nullptr),
	alternateVersionCheckRequest(nullptr),
	usingAltUpdateServer(false),
	updateAvailable(false),
	authUser(0, "")
{
	//Read config
	std::ifstream configFile;
	configFile.open("powder.pref", std::ios::binary);
	if (configFile)
	{
		try
		{
			preferences.clear();
			configFile >> preferences;
			int ID = preferences["User"]["ID"].asInt();
			ByteString Username = preferences["User"]["Username"].asString();
			ByteString SessionID = preferences["User"]["SessionID"].asString();
			ByteString SessionKey = preferences["User"]["SessionKey"].asString();
			ByteString Elevation = preferences["User"]["Elevation"].asString();

			authUser.UserID = ID;
			authUser.Username = Username;
			authUser.SessionID = SessionID;
			authUser.SessionKey = SessionKey;
			if (Elevation == "Admin")
				authUser.UserElevation = User::ElevationAdmin;
			else if (Elevation == "Mod")
				authUser.UserElevation = User::ElevationModerator;
			else
				authUser.UserElevation = User::ElevationNone;
		}
		catch (std::exception &e)
		{

		}
		configFile.close();
		firstRun = false;
	}
	else
		firstRun = true;
}

void Client::Initialise(ByteString proxy, ByteString cafile, ByteString capath, bool disableNetwork)
{
#if !defined(FONTEDITOR) && !defined(RENDERER)
	if (GetPrefBool("version.update", false))
	{
		SetPref("version.update", false);
		update_finish();
	}
#endif

#ifndef NOHTTP
	if (!disableNetwork)
		http::RequestManager::Ref().Initialise(proxy, cafile, capath);
#endif

	//Read stamps library
	std::ifstream stampsLib;
	stampsLib.open(STAMPS_DIR PATH_SEP "stamps.def", std::ios::binary);
	while (!stampsLib.eof())
	{
		char data[11];
		memset(data, 0, 11);
		stampsLib.read(data, 10);
		if(!data[0])
			break;
		stampIDs.push_back(data);
	}
	stampsLib.close();

	//Begin version check
	versionCheckRequest = new http::Request(SCHEME SERVER "/Startup.json");

	if (authUser.UserID)
	{
		versionCheckRequest->AuthHeaders(ByteString::Build(authUser.UserID), authUser.SessionID);
	}
	versionCheckRequest->Start();

#ifdef UPDATESERVER
	// use an alternate update server
	alternateVersionCheckRequest = new http::Request(SCHEME UPDATESERVER "/Startup.json");
	usingAltUpdateServer = true;
	if (authUser.UserID)
	{
		alternateVersionCheckRequest->AuthHeaders(authUser.Username, "");
	}
	alternateVersionCheckRequest->Start();
#endif
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
	if (versionCheckRequest)
	{
		if (CheckUpdate(versionCheckRequest, true))
			versionCheckRequest = nullptr;
	}
	if (alternateVersionCheckRequest)
	{
		if (CheckUpdate(alternateVersionCheckRequest, false))
			alternateVersionCheckRequest = nullptr;
	}
}

bool Client::CheckUpdate(http::Request *updateRequest, bool checkSession)
{
	//Check status on version check request
	if (updateRequest->CheckDone())
	{
		int status;
		ByteString data = updateRequest->Finish(&status);

		if (checkSession && status == 618)
		{
			AddServerNotification({ "Failed to load SSL certificates", SCHEME "powdertoy.co.uk/FAQ.html" });
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

#ifndef IGNORE_UPDATES
					//Check for updates
					Json::Value versions = objDocument["Updates"];
#ifndef SNAPSHOT
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
#endif

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

#if defined(SNAPSHOT) || MOD_ID > 0
					Json::Value snapshotVersion = versions["Snapshot"];
					int snapshotSnapshot = snapshotVersion["Snapshot"].asInt();
					ByteString snapshotFile = snapshotVersion["File"].asString();
					String snapshotChangelog = ByteString(snapshotVersion["Changelog"].asString()).FromUtf8();
					if (snapshotSnapshot > SNAPSHOT_ID)
					{
						updateAvailable = true;
						updateInfo = UpdateInfo(snapshotSnapshot, snapshotFile, snapshotChangelog, UpdateInfo::Snapshot);
					}
#endif

					if(updateAvailable)
					{
						notifyUpdateAvailable();
					}
#endif
				}
			}
			catch (std::exception & e)
			{
				//Do nothing
			}
		}
		return true;
	}
	return false;
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

void Client::WritePrefs()
{
	std::ofstream configFile;
	configFile.open("powder.pref", std::ios::trunc);

	if (configFile)
	{
		if (authUser.UserID)
		{
			preferences["User"]["ID"] = authUser.UserID;
			preferences["User"]["SessionID"] = authUser.SessionID;
			preferences["User"]["SessionKey"] = authUser.SessionKey;
			preferences["User"]["Username"] = authUser.Username;
			if (authUser.UserElevation == User::ElevationAdmin)
				preferences["User"]["Elevation"] = "Admin";
			else if (authUser.UserElevation == User::ElevationModerator)
				preferences["User"]["Elevation"] = "Mod";
			else
				preferences["User"]["Elevation"] = "None";
		}
		else
		{
			preferences["User"] = Json::nullValue;
		}
		configFile << preferences;

		configFile.close();
	}
}

void Client::Shutdown()
{
	if (versionCheckRequest)
	{
		versionCheckRequest->Cancel();
	}
	if (alternateVersionCheckRequest)
	{
		alternateVersionCheckRequest->Cancel();
	}

#ifndef NOHTTP
	http::RequestManager::Ref().Shutdown();
#endif

	//Save config
	WritePrefs();
}

Client::~Client()
{
}


void Client::SetAuthUser(User user)
{
	authUser = user;
	WritePrefs();
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
#if defined(SNAPSHOT) || defined(BETA) || defined(DEBUG) || MOD_ID > 0
		else if (fromNewerVersion && save.GetPublished())
		{
			lastError = "Cannot publish save, incompatible with latest release version.";
			return RequestFailure;
		}
#endif

		data = http::Request::SimpleAuth(SCHEME SERVER "/Save.api", &dataStatus, userID, authUser.SessionID, {
			{ "Name", save.GetName().ToUtf8() },
			{ "Description", save.GetDescription().ToUtf8() },
			{ "Data:save.bin", ByteString(gameData.begin(), gameData.end()) },
			{ "Publish", save.GetPublished() ? "Public" : "Private" },
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
	for (std::list<ByteString>::iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator)
	{
		if((*iterator) == stampID)
		{
			stampIDs.erase(iterator);
			break;
		}
	}
	stampIDs.push_front(stampID);
	updateStamps();
}

SaveFile * Client::GetStamp(ByteString stampID)
{
	ByteString stampFile = ByteString(STAMPS_DIR PATH_SEP + stampID + ".stm");
	SaveFile *saveFile = LoadSaveFile(stampFile);
	if (!saveFile)
		saveFile = LoadSaveFile(stampID);
	else
		saveFile->SetDisplayName(stampID.FromUtf8());
	return saveFile;
}

void Client::DeleteStamp(ByteString stampID)
{
	for (std::list<ByteString>::iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator)
	{
		if ((*iterator) == stampID)
		{
			ByteString stampFilename = ByteString::Build(STAMPS_DIR, PATH_SEP, stampID, ".stm");
			remove(stampFilename.c_str());
			stampIDs.erase(iterator);
			break;
		}
	}

	updateStamps();
}

ByteString Client::AddStamp(GameSave * saveData)
{
	unsigned t=(unsigned)time(NULL);
	if (lastStampTime!=t)
	{
		lastStampTime=t;
		lastStampName=0;
	}
	else
		lastStampName++;
	ByteString saveID = ByteString::Build(Format::Hex(Format::Width(lastStampTime, 8)), Format::Hex(Format::Width(lastStampName, 2)));
	ByteString filename = STAMPS_DIR PATH_SEP + saveID + ".stm";

	Platform::MakeDirectory(STAMPS_DIR);

	Json::Value stampInfo;
	stampInfo["type"] = "stamp";
	stampInfo["username"] = authUser.Username;
	stampInfo["name"] = filename;
	stampInfo["date"] = (Json::Value::UInt64)time(NULL);
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

	stampIDs.push_front(saveID);

	updateStamps();

	return saveID;
}

void Client::updateStamps()
{
	Platform::MakeDirectory(STAMPS_DIR);

	std::ofstream stampsStream;
	stampsStream.open(ByteString(STAMPS_DIR PATH_SEP "stamps.def").c_str(), std::ios::binary);
	for (std::list<ByteString>::const_iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator)
	{
		stampsStream.write((*iterator).c_str(), 10);
	}
	stampsStream.write("\0", 1);
	stampsStream.close();
	return;
}

void Client::RescanStamps()
{
	stampIDs.clear();
	for (auto &stamp : Platform::DirectorySearch("stamps", "", { ".stm" }))
	{
		if (stamp.size() == 14)
		{
			stampIDs.push_front(stamp.Substr(0, 10));
		}
	}
	stampIDs.sort(std::greater<ByteString>());
	updateStamps();
}

int Client::GetStampsCount()
{
	return stampIDs.size();
}

std::vector<ByteString> Client::GetStamps(int start, int count)
{
	int size = (int)stampIDs.size();
	if (start+count > size)
	{
		if(start > size)
			return std::vector<ByteString>();
		count = size-start;
	}

	std::vector<ByteString> stampRange;
	int index = 0;
	for (std::list<ByteString>::const_iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator, ++index)
	{
		if(index>=start && index < start+count)
			stampRange.push_back(*iterator);
	}
	return stampRange;
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
		data = http::Request::SimpleAuth(SCHEME SERVER "/Vote.api", &dataStatus, userIDText, authUser.SessionID, {
			{ "ID", saveIDText },
			{ "Action", direction == 1 ? "Up" : "Down" },
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
	int dataStatus;
	ByteString data;
	ByteString urlStr;
	if (saveDate)
		urlStr = ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, "_", saveDate, ".cps");
	else
		urlStr = ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, ".cps");

	data = http::Request::Simple(urlStr, &dataStatus);

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

	ByteString data;
	int dataStatus;
	data = http::Request::Simple("https://" SERVER "/Login.json", &dataStatus, {
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID);
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID, {
			{ "Comment", comment.ToUtf8() },
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
		data = http::Request::SimpleAuth(urlStream.Build(), &dataStatus, userID, authUser.SessionID);
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID, {
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID);
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID, {
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

SaveInfo * Client::GetSave(int saveID, int saveDate)
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
		
		data = http::Request::SimpleAuth(urlStream.Build(), &dataStatus, userID, authUser.SessionID);
	}
	else
	{
		data = http::Request::Simple(urlStream.Build(), &dataStatus);
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

			SaveInfo * tempSave = new SaveInfo(tempID, tempCreatedDate, tempUpdatedDate, tempScoreUp,
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
			return NULL;
		}
	}
	else
	{
		lastError = http::StatusText(dataStatus);
	}
	return NULL;
}

SaveFile * Client::LoadSaveFile(ByteString filename)
{
	ByteString err;
	SaveFile *file = nullptr;
	if (Platform::FileExists(filename))
	{
		file = new SaveFile(filename);
		try
		{
			std::vector<char> data;
			if (Platform::ReadFile(data, filename))
			{
				file->SetGameSave(new GameSave(std::move(data)));
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
#ifdef LUACONSOLE
		luacon_ci->SetLastError(err.FromUtf8());
#endif
	}
	return file;
}

std::vector<std::pair<ByteString, int> > * Client::GetTags(int start, int count, String query, int & resultCount)
{
	lastError = "";
	resultCount = 0;
	std::vector<std::pair<ByteString, int> > * tagArray = new std::vector<std::pair<ByteString, int> >();
	ByteStringBuilder urlStream;
	ByteString data;
	int dataStatus;
	urlStream << SCHEME << SERVER << "/Browse/Tags.json?Start=" << start << "&Count=" << count;
	if(query.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << format::URLEncode(query.ToUtf8());
	}

	data = http::Request::Simple(urlStream.Build(), &dataStatus);
	if(dataStatus == 200 && data.size())
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

			resultCount = objDocument["TagTotal"].asInt();
			Json::Value tagsArray = objDocument["Tags"];
			for (Json::UInt j = 0; j < tagsArray.size(); j++)
			{
				int tagCount = tagsArray[j]["Count"].asInt();
				ByteString tag = tagsArray[j]["Tag"].asString();
				tagArray->push_back(std::pair<ByteString, int>(tag, tagCount));
			}
		}
		catch (std::exception & e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
		}
	}
	else
	{
		lastError = http::StatusText(dataStatus);
	}
	return tagArray;
}

std::vector<SaveInfo*> * Client::SearchSaves(int start, int count, String query, ByteString sort, ByteString category, int & resultCount)
{
	lastError = "";
	resultCount = 0;
	std::vector<SaveInfo*> * saveArray = new std::vector<SaveInfo*>();
	ByteStringBuilder urlStream;
	ByteString data;
	int dataStatus;
	urlStream << SCHEME << SERVER << "/Browse.json?Start=" << start << "&Count=" << count;
	if(query.length() || sort.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << format::URLEncode(query.ToUtf8());
		if(sort == "date")
		{
			if(query.length())
				urlStream << format::URLEncode(" ");
			urlStream << format::URLEncode("sort:") << format::URLEncode(sort);
		}
	}
	if(category.length())
	{
		urlStream << "&Category=" << format::URLEncode(category);
	}
	if(authUser.UserID)
	{
		ByteString userID = ByteString::Build(authUser.UserID);
		data = http::Request::SimpleAuth(urlStream.Build(), &dataStatus, userID, authUser.SessionID);
	}
	else
	{
		data = http::Request::Simple(urlStream.Build(), &dataStatus);
	}
	ParseServerReturn(data, dataStatus, true);
	if (dataStatus == 200 && data.size())
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

			resultCount = objDocument["Count"].asInt();
			Json::Value savesArray = objDocument["Saves"];
			for (Json::UInt j = 0; j < savesArray.size(); j++)
			{
				int tempID = savesArray[j]["ID"].asInt();
				int tempCreatedDate = savesArray[j]["Created"].asInt();
				int tempUpdatedDate = savesArray[j]["Updated"].asInt();
				int tempScoreUp = savesArray[j]["ScoreUp"].asInt();
				int tempScoreDown = savesArray[j]["ScoreDown"].asInt();
				ByteString tempUsername = savesArray[j]["Username"].asString();
				String tempName = ByteString(savesArray[j]["Name"].asString()).FromUtf8();
				int tempVersion = savesArray[j]["Version"].asInt();
				bool tempPublished = savesArray[j]["Published"].asBool();
				SaveInfo * tempSaveInfo = new SaveInfo(tempID, tempCreatedDate, tempUpdatedDate, tempScoreUp, tempScoreDown, tempUsername, tempName);
				tempSaveInfo->Version = tempVersion;
				tempSaveInfo->SetPublished(tempPublished);
				saveArray->push_back(tempSaveInfo);
			}
		}
		catch (std::exception &e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
		}
	}
	return saveArray;
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID);
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
		data = http::Request::SimpleAuth(url, &dataStatus, userID, authUser.SessionID);
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

// powder.pref preference getting / setting functions

// Recursively go down the json to get the setting we want
Json::Value Client::GetPref(Json::Value root, ByteString prop, Json::Value defaultValue)
{
	try
	{
		if(ByteString::Split split = prop.SplitBy('.'))
			return GetPref(root[split.Before()], split.After(), defaultValue);
		else
			return root.get(prop, defaultValue);
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

ByteString Client::GetPrefByteString(ByteString prop, ByteString defaultValue)
{
	try
	{
		return GetPref(preferences, prop, defaultValue).asString();
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

String Client::GetPrefString(ByteString prop, String defaultValue)
{
	try
	{
		return ByteString(GetPref(preferences, prop, defaultValue.ToUtf8()).asString()).FromUtf8(false);
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

double Client::GetPrefNumber(ByteString prop, double defaultValue)
{
	try
	{
		return GetPref(preferences, prop, defaultValue).asDouble();
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

int Client::GetPrefInteger(ByteString prop, int defaultValue)
{
	try
	{
		return GetPref(preferences, prop, defaultValue).asInt();
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

unsigned int Client::GetPrefUInteger(ByteString prop, unsigned int defaultValue)
{
	try
	{
		return GetPref(preferences, prop, defaultValue).asUInt();
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

bool Client::GetPrefBool(ByteString prop, bool defaultValue)
{
	try
	{
		return GetPref(preferences, prop, defaultValue).asBool();
	}
	catch (std::exception & e)
	{
		return defaultValue;
	}
}

std::vector<ByteString> Client::GetPrefByteStringArray(ByteString prop)
{
	try
	{
		std::vector<ByteString> ret;
		Json::Value arr = GetPref(preferences, prop);
		for (int i = 0; i < (int)arr.size(); i++)
			ret.push_back(arr[i].asString());
		return ret;
	}
	catch (std::exception & e)
	{

	}
	return std::vector<ByteString>();
}

std::vector<String> Client::GetPrefStringArray(ByteString prop)
{
	try
	{
		std::vector<String> ret;
		Json::Value arr = GetPref(preferences, prop);
		for (int i = 0; i < (int)arr.size(); i++)
			ret.push_back(ByteString(arr[i].asString()).FromUtf8(false));
		return ret;
	}
	catch (std::exception & e)
	{

	}
	return std::vector<String>();
}

std::vector<double> Client::GetPrefNumberArray(ByteString prop)
{
	try
	{
		std::vector<double> ret;
		Json::Value arr = GetPref(preferences, prop);
		for (int i = 0; i < (int)arr.size(); i++)
			ret.push_back(arr[i].asDouble());
		return ret;
	}
	catch (std::exception & e)
	{

	}
	return std::vector<double>();
}

std::vector<int> Client::GetPrefIntegerArray(ByteString prop)
{
	try
	{
		std::vector<int> ret;
		Json::Value arr = GetPref(preferences, prop);
		for (int i = 0; i < (int)arr.size(); i++)
			ret.push_back(arr[i].asInt());
		return ret;
	}
	catch (std::exception & e)
	{

	}
	return std::vector<int>();
}

std::vector<unsigned int> Client::GetPrefUIntegerArray(ByteString prop)
{
	try
	{
		std::vector<unsigned int> ret;
		Json::Value arr = GetPref(preferences, prop);
		for (int i = 0; i < (int)arr.size(); i++)
			ret.push_back(arr[i].asUInt());
		return ret;
	}
	catch (std::exception & e)
	{

	}
	return std::vector<unsigned int>();
}

std::vector<bool> Client::GetPrefBoolArray(ByteString prop)
{
	try
	{
		std::vector<bool> ret;
		Json::Value arr = GetPref(preferences, prop);
		for (int i = 0; i < (int)arr.size(); i++)
			ret.push_back(arr[i].asBool());
		return ret;
	}
	catch (std::exception & e)
	{

	}
	return std::vector<bool>();
}

// Helper preference setting function.
// To actually save any changes to preferences, we need to directly do preferences[property] = thing
// any other way will set the value of a copy of preferences, not the original
// This function will recursively go through and create an object with the property we wanted set,
// and return it to SetPref to do the actual setting
Json::Value Client::SetPrefHelper(Json::Value root, ByteString prop, Json::Value value)
{
	if(ByteString::Split split = prop.SplitBy('.'))
	{
		Json::Value toSet = GetPref(root, split.Before());
		toSet = SetPrefHelper(toSet, split.After(), value);
		root[split.Before()] = toSet;
	}
	else
		root[prop] = value;
	return root;
}

void Client::SetPref(ByteString prop, Json::Value value)
{
	try
	{
		if(ByteString::Split split = prop.SplitBy('.'))
			preferences[split.Before()] = SetPrefHelper(preferences[split.Before()], split.After(), value);
		else
			preferences[prop] = value;
		WritePrefs();
	}
	catch (std::exception & e)
	{

	}
}

void Client::SetPref(ByteString prop, std::vector<Json::Value> value)
{
	try
	{
		Json::Value arr;
		for (int i = 0; i < (int)value.size(); i++)
		{
			arr.append(value[i]);
		}
		SetPref(prop, arr);
	}
	catch (std::exception & e)
	{

	}
}

void Client::SetPrefUnicode(ByteString prop, String value)
{
	SetPref(prop, value.ToUtf8());
}

bool Client::DoInstallation()
{
	bool ok = true;
#if defined(WIN)
	auto deleteKey = [](ByteString path) {
		RegDeleteKeyW(HKEY_CURRENT_USER, Platform::WinWiden(path).c_str());
	};
	auto createKey = [](ByteString path, ByteString value, ByteString extraKey = {}, ByteString extraValue = {}) {
		auto ok = true;
		auto wPath = Platform::WinWiden(path);
		auto wValue = Platform::WinWiden(value);
		auto wExtraKey = Platform::WinWiden(extraKey);
		auto wExtraValue = Platform::WinWiden(extraValue);
		HKEY k;
		ok = ok && RegCreateKeyExW(HKEY_CURRENT_USER, wPath.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &k, NULL) == ERROR_SUCCESS;
		ok = ok && RegSetValueExW(k, NULL, 0, REG_SZ, reinterpret_cast<const BYTE *>(wValue.c_str()), (wValue.size() + 1) * 2) == ERROR_SUCCESS;
		if (wExtraKey.size())
		{
			ok = ok && RegSetValueExW(k, wExtraKey.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(wExtraValue.c_str()), (wExtraValue.size() + 1) * 2) == ERROR_SUCCESS;
		}
		RegCloseKey(k);
		return ok;
	};

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	auto exe = Platform::ExecutableName();
#ifndef IDI_DOC_ICON
	// make this fail so I don't remove #include "resource.h" again and get away with it
# error where muh IDI_DOC_ICON D:
#endif
	auto icon = exe + ",-" MTOS(IDI_DOC_ICON);
	auto path = Platform::GetCwd();
	auto open = ByteString::Build("\"", exe, "\" ddir \"", path, "\" \"file://%1\"");
	auto ptsave = ByteString::Build("\"", exe, "\" ddir \"", path, "\" \"%1\"");
	deleteKey("Software\\Classes\\ptsave");
	deleteKey("Software\\Classes\\.cps");
	deleteKey("Software\\Classes\\.stm");
	deleteKey("Software\\Classes\\PowderToySave");
	ok = ok && createKey("Software\\Classes\\ptsave", "Powder Toy Save", "URL Protocol", "");
	ok = ok && createKey("Software\\Classes\\ptsave\\DefaultIcon", icon);
	ok = ok && createKey("Software\\Classes\\ptsave\\shell\\open\\command", ptsave);
	ok = ok && createKey("Software\\Classes\\.cps", "PowderToySave");
	ok = ok && createKey("Software\\Classes\\.stm", "PowderToySave");
	ok = ok && createKey("Software\\Classes\\PowderToySave", "Powder Toy Save");
	ok = ok && createKey("Software\\Classes\\PowderToySave\\DefaultIcon", icon);
	ok = ok && createKey("Software\\Classes\\PowderToySave\\shell\\open\\command", open);
	IShellLinkW *shellLink = NULL;
	IPersistFile *shellLinkPersist = NULL;
	wchar_t programsPath[MAX_PATH];
	ok = ok && SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, programsPath) == S_OK;
	ok = ok && CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID *)&shellLink) == S_OK;
	ok = ok && shellLink->SetPath(Platform::WinWiden(exe).c_str()) == S_OK;
	ok = ok && shellLink->SetWorkingDirectory(Platform::WinWiden(path).c_str()) == S_OK;
	ok = ok && shellLink->SetDescription(Platform::WinWiden(APPNAME).c_str()) == S_OK;
	ok = ok && shellLink->QueryInterface(IID_IPersistFile, (LPVOID *)&shellLinkPersist) == S_OK;
	ok = ok && shellLinkPersist->Save(Platform::WinWiden(Platform::WinNarrow(programsPath) + "\\" APPNAME ".lnk").c_str(), TRUE) == S_OK;
	if (shellLinkPersist)
	{
		shellLinkPersist->Release();
	}
	if (shellLink)
	{
		shellLink->Release();
	}
	CoUninitialize();
#elif defined(LIN)
	auto desktopEscapeString = [](ByteString str) {
		ByteString escaped;
		for (auto ch : str)
		{
			auto from = " " "\n" "\t" "\r" "\\";
			auto to   = "s"  "n"  "t"  "r" "\\";
			if (auto off = strchr(from, ch))
			{
				escaped.append(1, '\\');
				escaped.append(1, to[off - from]);
			}
			else
			{
				escaped.append(1, ch);
			}
		}
		return escaped;
	};
	auto desktopEscapeExec = [](ByteString str) {
		ByteString escaped;
		for (auto ch : str)
		{
			if (strchr(" \t\n\"\'\\><~|&;$*?#()`", ch))
			{
				escaped.append(1, '\\');
			}
			escaped.append(1, ch);
		}
		return escaped;
	};

	if (ok)
	{
		ByteString desktopData(powder_desktop, powder_desktop + powder_desktop_size);
		auto exe = Platform::ExecutableName();
		auto path = exe.SplitFromEndBy('/').Before();
		desktopData = desktopData.Substitute("Exec=" APPEXE, "Exec=" + desktopEscapeString(desktopEscapeExec(exe)));
		desktopData += ByteString::Build("Path=", desktopEscapeString(path), "\n");
		ByteString file = APPVENDOR "-" APPID ".desktop";
		ok = ok && Platform::WriteFile(std::vector<char>(desktopData.begin(), desktopData.end()), file);
		ok = ok && !system(ByteString::Build("xdg-desktop-menu install ", file).c_str());
		ok = ok && !system(ByteString::Build("xdg-mime default ", file, " application/vnd.powdertoy.save").c_str());
		ok = ok && !system(ByteString::Build("xdg-mime default ", file, " x-scheme-handler/ptsave").c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = APPVENDOR "-save.xml";
		ok = ok && Platform::WriteFile(std::vector<char>(save_xml, save_xml + save_xml_size), file);
		ok = ok && !system(ByteString::Build("xdg-mime install ", file).c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = APPVENDOR "-cps.png";
		ok = ok && Platform::WriteFile(std::vector<char>(icon_cps_png, icon_cps_png + icon_cps_png_size), file);
		ok = ok && !system(ByteString::Build("xdg-icon-resource install --noupdate --context mimetypes --size 64 ", file, " application-vnd.powdertoy.save").c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = APPVENDOR "-exe.png";
		ok = ok && Platform::WriteFile(std::vector<char>(icon_exe_png, icon_exe_png + icon_exe_png_size), file);
		ok = ok && !system(ByteString::Build("xdg-icon-resource install --noupdate --size 64 ", file, " " APPVENDOR "-" APPEXE).c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ok = ok && !system("xdg-icon-resource forceupdate");
	}
#else
	ok = false;
#endif
	return ok;
}
