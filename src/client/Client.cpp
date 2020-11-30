#include "Client.h"

#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <fstream>
#include <dirent.h>

#ifdef MACOSX
#include <mach-o/dyld.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef WIN
#define NOMINMAX
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "common/String.h"
#include "Config.h"
#include "Format.h"
#include "MD5.h"
#include "Platform.h"
#include "Update.h"

#include "ClientListener.h"

#include "graphics/Graphics.h"

#include "gui/preview/Comment.h"

#include "client/SaveInfo.h"
#include "client/SaveFile.h"
#include "client/GameSave.h"
#include "client/UserInfo.h"
#include "client/http/Request.h"
#include "client/http/RequestManager.h"


extern "C"
{
#if defined(WIN) && !defined(__GNUC__)
#include <io.h>
#else
#include <dirent.h>
#endif
}


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

void Client::Initialise(ByteString proxyString, bool disableNetwork)
{
	if (GetPrefBool("version.update", false))
	{
		SetPref("version.update", false);
		update_finish();
	}

#ifndef NOHTTP
	if (!disableNetwork)
		http::RequestManager::Ref().Initialise(proxyString);
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

bool Client::DoInstallation()
{
#if defined(WIN)
	int returnval;
	LONG rresult;
	HKEY newkey;
	ByteString currentfilename2 = Platform::ExecutableName();
	// this isn't necessary but I don't feel like c++ifying this code right now
	const char *currentfilename = currentfilename2.c_str();
	char *iconname = NULL;
	char *opencommand = NULL;
	char *protocolcommand = NULL;
	//char AppDataPath[MAX_PATH];
	char *AppDataPath = NULL;
	iconname = (char*)malloc(strlen(currentfilename)+6);
	sprintf(iconname, "%s,-102", currentfilename);

	//Create Roaming application data folder
	/*if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, AppDataPath)))
	{
		returnval = 0;
		goto finalise;
	}*/

	AppDataPath = _getcwd(NULL, 0);

	//Move Game executable into application data folder
	//TODO: Implement

	opencommand = (char*)malloc(strlen(currentfilename)+53+strlen(AppDataPath));
	protocolcommand = (char*)malloc(strlen(currentfilename)+53+strlen(AppDataPath));
	/*if((strlen(AppDataPath)+strlen(APPDATA_SUBDIR "\\Powder Toy"))<MAX_PATH)
	{
		strappend(AppDataPath, APPDATA_SUBDIR);
		_mkdir(AppDataPath);
		strappend(AppDataPath, "\\Powder Toy");
		_mkdir(AppDataPath);
	} else {
		returnval = 0;
		goto finalise;
	}*/
	sprintf(opencommand, "\"%s\" open \"%%1\" ddir \"%s\"", currentfilename, AppDataPath);
	sprintf(protocolcommand, "\"%s\" ddir \"%s\" ptsave \"%%1\"", currentfilename, AppDataPath);

	//Create protocol entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\ptsave", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"Powder Toy Save", strlen("Powder Toy Save")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, (LPCSTR)"URL Protocol", 0, REG_SZ, (LPBYTE)"", strlen("")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Set Protocol DefaultIcon
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\ptsave\\DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)iconname, strlen(iconname)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Set Protocol Launch command
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\ptsave\\shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)protocolcommand, strlen(protocolcommand)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Create extension entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\.cps", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"PowderToySave", strlen("PowderToySave")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\.stm", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"PowderToySave", strlen("PowderToySave")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Create program entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"Powder Toy Save", strlen("Powder Toy Save")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Set DefaultIcon
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave\\DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)iconname, strlen(iconname)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Set Launch command
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave\\shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)opencommand, strlen(opencommand)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	returnval = 1;
	finalise:

	free(iconname);
	free(opencommand);
	free(protocolcommand);

	return returnval;
#elif defined(LIN)
	#include "icondoc.h"

	int success = 1;
	ByteString filename = Platform::ExecutableName(), pathname = filename.SplitFromEndBy('/').Before();
	filename.Substitute('\'', "'\\''");
	filename = '\'' + filename + '\'';

	FILE *f;
	const char *mimedata =
"<?xml version=\"1.0\"?>\n"
"	<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n"
"	<mime-type type=\"application/vnd.powdertoy.save\">\n"
"		<comment>Powder Toy save</comment>\n"
"		<glob pattern=\"*.cps\"/>\n"
"		<glob pattern=\"*.stm\"/>\n"
"	</mime-type>\n"
"</mime-info>\n";
	f = fopen("powdertoy-save.xml", "wb");
	if (!f)
		return 0;
	fwrite(mimedata, 1, strlen(mimedata), f);
	fclose(f);

	const char *protocolfiledata_tmp =
"[Desktop Entry]\n"
"Type=Application\n"
"Name=Powder Toy\n"
"Comment=Physics sandbox game\n"
"MimeType=x-scheme-handler/ptsave;\n"
"NoDisplay=true\n"
"Categories=Game;Simulation\n"
"Icon=powdertoy.png\n";
	ByteString protocolfiledata = ByteString::Build(protocolfiledata_tmp, "Exec=", filename, " ptsave %u\nPath=", pathname, "\n");
	f = fopen("powdertoy-tpt-ptsave.desktop", "wb");
	if (!f)
		return 0;
	fwrite(protocolfiledata.c_str(), 1, protocolfiledata.size(), f);
	fclose(f);
	success = system("xdg-desktop-menu install powdertoy-tpt-ptsave.desktop");

	const char *desktopopenfiledata_tmp =
"[Desktop Entry]\n"
"Type=Application\n"
"Name=Powder Toy\n"
"Comment=Physics sandbox game\n"
"MimeType=application/vnd.powdertoy.save;\n"
"NoDisplay=true\n"
"Categories=Game;Simulation\n"
"Icon=powdertoy.png\n";
	ByteString desktopopenfiledata = ByteString::Build(desktopopenfiledata_tmp, "Exec=", filename, " open %f\nPath=", pathname, "\n");
	f = fopen("powdertoy-tpt-open.desktop", "wb");
	if (!f)
		return 0;
	fwrite(desktopopenfiledata.c_str(), 1, desktopopenfiledata.size(), f);
	fclose(f);
	success = system("xdg-mime install powdertoy-save.xml") && success;
	success = system("xdg-desktop-menu install powdertoy-tpt-open.desktop") && success;

	const char *desktopfiledata_tmp =
"[Desktop Entry]\n"
"Version=1.0\n"
"Encoding=UTF-8\n"
"Name=Powder Toy\n"
"Type=Application\n"
"Comment=Physics sandbox game\n"
"Categories=Game;Simulation\n"
"Icon=powdertoy.png\n";
	ByteString desktopfiledata = ByteString::Build(desktopfiledata_tmp, "Exec=", filename, "\nPath=", pathname, "\n");
	f = fopen("powdertoy-tpt.desktop", "wb");
	if (!f)
		return 0;
	fwrite(desktopfiledata.c_str(), 1, desktopfiledata.size(), f);
	fclose(f);
	success = system("xdg-desktop-menu install powdertoy-tpt.desktop") && success;

	f = fopen("powdertoy-save-32.png", "wb");
	if (!f)
		return 0;
	fwrite(icon_doc_32_png, 1, sizeof(icon_doc_32_png), f);
	fclose(f);
	f = fopen("powdertoy-save-16.png", "wb");
	if (!f)
		return 0;
	fwrite(icon_doc_16_png, 1, sizeof(icon_doc_16_png), f);
	fclose(f);
	f = fopen("powdertoy.png", "wb");
	if (!f)
		return 0;
	fwrite(icon_desktop_48_png, 1, sizeof(icon_desktop_48_png), f);
	fclose(f);
	success = system("xdg-icon-resource install --noupdate --context mimetypes --size 32 powdertoy-save-32.png application-vnd.powdertoy.save") && success;
	success = system("xdg-icon-resource install --noupdate --context mimetypes --size 16 powdertoy-save-16.png application-vnd.powdertoy.save") && success;
	success = system("xdg-icon-resource install --noupdate --novendor --size 48 powdertoy.png") && success;
	success = system("xdg-icon-resource forceupdate") && success;
	success = system("xdg-mime default powdertoy-tpt-open.desktop application/vnd.powdertoy.save") && success;
	success = system("xdg-mime default powdertoy-tpt-ptsave.desktop x-scheme-handler/ptsave") && success;
	unlink("powdertoy.png");
	unlink("powdertoy-save-32.png");
	unlink("powdertoy-save-16.png");
	unlink("powdertoy-save.xml");
	unlink("powdertoy-tpt.desktop");
	unlink("powdertoy-tpt-open.desktop");
	unlink("powdertoy-tpt-ptsave.desktop");
	return !success;
#elif defined MACOSX
	return false;
#endif
}

std::vector<ByteString> Client::DirectorySearch(ByteString directory, ByteString search, ByteString extension)
{
	std::vector<ByteString> extensions;
	extensions.push_back(extension);
	return DirectorySearch(directory, search.ToUpper(), extensions);
}

std::vector<ByteString> Client::DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions)
{
	//Get full file listing
	//Normalise directory string, ensure / or \ is present
	if(*directory.rbegin() != '/' && *directory.rbegin() != '\\')
		directory += PATH_SEP;
	std::vector<ByteString> directoryList;
#if defined(WIN) && !defined(__GNUC__)
	//Windows
	struct _finddata_t currentFile;
	intptr_t findFileHandle;
	ByteString fileMatch = directory + "*.*";
	findFileHandle = _findfirst(fileMatch.c_str(), &currentFile);
	if (findFileHandle == -1L)
	{
#ifdef DEBUG
		printf("Unable to open directory: %s\n", directory.c_str());
#endif
		return std::vector<ByteString>();
	}
	do
	{
		ByteString currentFileName = ByteString(currentFile.name);
		if(currentFileName.length()>4)
			directoryList.push_back(directory+currentFileName);
	}
	while (_findnext(findFileHandle, &currentFile) == 0);
	_findclose(findFileHandle);
#else
	//Linux or MinGW
	struct dirent * directoryEntry;
	DIR *directoryHandle = opendir(directory.c_str());
	if(!directoryHandle)
	{
#ifdef DEBUG
		printf("Unable to open directory: %s\n", directory.c_str());
#endif
		return std::vector<ByteString>();
	}
	while ((directoryEntry = readdir(directoryHandle)))
	{
		ByteString currentFileName = ByteString(directoryEntry->d_name);
		if(currentFileName.length()>4)
			directoryList.push_back(directory+currentFileName);
	}
	closedir(directoryHandle);
#endif

	std::vector<ByteString> searchResults;
	for(std::vector<ByteString>::iterator iter = directoryList.begin(), end = directoryList.end(); iter != end; ++iter)
	{
		ByteString filename = *iter, tempfilename = *iter;
		bool extensionMatch = !extensions.size();
		for(std::vector<ByteString>::iterator extIter = extensions.begin(), extEnd = extensions.end(); extIter != extEnd; ++extIter)
		{
			if(filename.EndsWith(*extIter))
			{
				extensionMatch = true;
				tempfilename = filename.SubstrFromEnd(0, (*extIter).size()).ToUpper();
				break;
			}
		}
		bool searchMatch = !search.size();
		if(search.size() && tempfilename.Contains(search))
			searchMatch = true;

		if(searchMatch && extensionMatch)
			searchResults.push_back(filename);
	}

	//Filter results
	return searchResults;
}

int Client::MakeDirectory(const char * dirName)
{
#ifdef WIN
	return _mkdir(dirName);
#else
	return mkdir(dirName, 0755);
#endif
}

bool Client::WriteFile(std::vector<unsigned char> fileData, ByteString filename)
{
	bool saveError = false;
	try
	{
		std::ofstream fileStream;
		fileStream.open(filename, std::ios::binary);
		if(fileStream.is_open())
		{
			fileStream.write((char*)&fileData[0], fileData.size());
			fileStream.close();
		}
		else
			saveError = true;
	}
	catch (std::exception & e)
	{
		std::cerr << "WriteFile:" << e.what() << std::endl;
		saveError = true;
	}
	return saveError;
}

bool Client::FileExists(ByteString filename)
{
	bool exists = false;
	try
	{
		std::ifstream fileStream;
		fileStream.open(filename, std::ios::binary);
		if(fileStream.is_open())
		{
			exists = true;
			fileStream.close();
		}
	}
	catch (std::exception & e)
	{
		exists = false;
	}
	return exists;
}

bool Client::WriteFile(std::vector<char> fileData, ByteString filename)
{
	bool saveError = false;
	try
	{
		std::ofstream fileStream;
		fileStream.open(filename, std::ios::binary);
		if(fileStream.is_open())
		{
			fileStream.write(&fileData[0], fileData.size());
			fileStream.close();
		}
		else
			saveError = true;
	}
	catch (std::exception & e)
	{
		std::cerr << "WriteFile:" << e.what() << std::endl;
		saveError = true;
	}
	return saveError;
}

std::vector<unsigned char> Client::ReadFile(ByteString filename)
{
	try
	{
		std::ifstream fileStream;
		fileStream.open(filename, std::ios::binary);
		if(fileStream.is_open())
		{
			fileStream.seekg(0, std::ios::end);
			size_t fileSize = fileStream.tellg();
			fileStream.seekg(0);

			unsigned char * tempData = new unsigned char[fileSize];
			fileStream.read((char *)tempData, fileSize);
			fileStream.close();

			std::vector<unsigned char> fileData;
			fileData.insert(fileData.end(), tempData, tempData+fileSize);
			delete[] tempData;

			return fileData;
		}
		else
		{
			return std::vector<unsigned char>();
		}
	}
	catch(std::exception & e)
	{
		std::cerr << "Readfile: " << e.what() << std::endl;
		throw;
	}
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

		if (status != 200)
		{
			//free(data);
			if (usingAltUpdateServer && !checkSession)
				this->messageOfTheDay = String::Build("HTTP Error ", status, " while checking for updates: ", http::StatusText(status));
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
	notifyAuthUserChanged();
}

User Client::GetAuthUser()
{
	return authUser;
}

RequestStatus Client::UploadSave(SaveInfo & save)
{
	lastError = "";
	unsigned int gameDataLength;
	char * gameData = NULL;
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

		gameData = save.GetGameSave()->Serialise(gameDataLength);

		if (!gameData)
		{
			lastError = "Cannot serialize game save";
			return RequestFailure;
		}
#if defined(SNAPSHOT) || defined(BETA) || defined(DEBUG) || MOD_ID > 0
		else if (save.gameSave->fromNewerVersion && save.GetPublished())
		{
			lastError = "Cannot publish save, incompatible with latest release version.";
			return RequestFailure;
		}
#endif

		data = http::Request::SimpleAuth(SCHEME SERVER "/Save.api", &dataStatus, userID, authUser.SessionID, {
			{ "Name", save.GetName().ToUtf8() },
			{ "Description", save.GetDescription().ToUtf8() },
			{ "Data:save.bin", ByteString(gameData, gameData + gameDataLength) },
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
	delete[] gameData;
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

	MakeDirectory(STAMPS_DIR);

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

	unsigned int gameDataLength;
	char * gameData = saveData->Serialise(gameDataLength);
	if (gameData == NULL)
		return "";

	std::ofstream stampStream;
	stampStream.open(filename.c_str(), std::ios::binary);
	stampStream.write((const char *)gameData, gameDataLength);
	stampStream.close();

	delete[] gameData;

	stampIDs.push_front(saveID);

	updateStamps();

	return saveID;
}

void Client::updateStamps()
{
	MakeDirectory(STAMPS_DIR);

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
	DIR * directory;
	struct dirent * entry;
	directory = opendir("stamps");
	if (directory != NULL)
	{
		stampIDs.clear();
		while ((entry = readdir(directory)))
		{
			ByteString name = entry->d_name;
			if(name != ".." && name != "." && name.EndsWith(".stm") && name.size() == 14)
				stampIDs.push_front(name.Substr(0, 10));
		}
		closedir(directory);
		stampIDs.sort(std::greater<ByteString>());
		updateStamps();
	}
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

std::vector<unsigned char> Client::GetSaveData(int saveID, int saveDate)
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
		return std::vector<unsigned char>(data.begin(), data.end());
	}
	return std::vector<unsigned char>();
}

LoginStatus Client::Login(ByteString username, ByteString password, User & user)
{
	lastError = "";
	char passwordHash[33];
	char totalHash[33];

	user.UserID = 0;
	user.Username = "";
	user.SessionID = "";
	user.SessionKey = "";

	//Doop
	md5_ascii(passwordHash, (const unsigned char *)password.c_str(), password.length());
	passwordHash[32] = 0;
	ByteString total = ByteString::Build(username, "-", passwordHash);
	md5_ascii(totalHash, (const unsigned char *)(total.c_str()), total.size());
	totalHash[32] = 0;

	ByteString data;
	int dataStatus;
	data = http::Request::Simple(SCHEME SERVER "/Login.json", &dataStatus, {
		{ "Username", username },
		{ "Hash", totalHash },
	});

	RequestStatus ret = ParseServerReturn(data, dataStatus, true);
	if (ret == RequestOkay)
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

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

			user.Username = username;
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
	if (!FileExists(filename))
		return nullptr;
	SaveFile * file = new SaveFile(filename);
	try
	{
		GameSave * tempSave = new GameSave(ReadFile(filename));
		file->SetGameSave(tempSave);
	}
	catch (ParseException & e)
	{
		std::cerr << "Client: Invalid save file '" << filename << "': " << e.what() << std::endl;
		file->SetLoadingError(ByteString(e.what()).FromUtf8());
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
