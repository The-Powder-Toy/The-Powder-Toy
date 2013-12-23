#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <time.h>
#include <stdio.h>
#include <deque>
#include <fstream>
#include <dirent.h>

#ifdef MACOSX
#include <mach-o/dyld.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef WIN
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Config.h"
#include "Format.h"
#include "Client.h"
#include "MD5.h"
#include "graphics/Graphics.h"
#include "Misc.h"
#include "Update.h"
#include "HTTP.h"

#include "simulation/SaveRenderer.h"
#include "gui/interface/Point.h"
#include "client/SaveInfo.h"
#include "client/SaveFile.h"
#include "client/GameSave.h"
#include "client/UserInfo.h"
#include "gui/search/Thumbnail.h"
#include "gui/preview/Comment.h"
#include "ClientListener.h"
#include "requestbroker/RequestBroker.h"
#include "requestbroker/WebRequest.h"
#include "requestbroker/APIRequest.h"
#include "requestbroker/APIResultParser.h"

#include "cajun/reader.h"
#include "cajun/writer.h"

extern "C"
{
#if defined(WIN) && !defined(__GNUC__)
#include <io.h>
#else
#include <dirent.h>
#endif
#ifdef MACOSX
char * readUserPreferences();
void writeUserPreferences(const char * prefData);
#endif
}


Client::Client():
	authUser(0, ""),
	updateAvailable(false),
	versionCheckRequest(NULL),
	messageOfTheDay("")
{
	int i = 0;
	for(i = 0; i < THUMB_CACHE_SIZE; i++)
	{
		thumbnailCache[i] = NULL;
	}
	for(i = 0; i < IMGCONNS; i++)
	{
		activeThumbRequests[i] = NULL;
		activeThumbRequestTimes[i] = 0;
		activeThumbRequestCompleteTimes[i] = 0;
	}

	//Read config
#ifdef MACOSX
	char * prefData = readUserPreferences();
	std::stringstream configFile(prefData);
	free(prefData);
#else
	std::ifstream configFile;
	configFile.open("powder.pref", std::ios::binary);
#endif
	if(configFile)
	{
		int fsize = configFile.tellg();
		configFile.seekg(0, std::ios::end);
		fsize = configFile.tellg() - (std::streampos)fsize;
		configFile.seekg(0, std::ios::beg);
		if(fsize)
		{
			try
			{
				json::Reader::Read(configDocument, configFile);
				authUser.ID = ((json::Number)(configDocument["User"]["ID"])).Value();
				authUser.SessionID = ((json::String)(configDocument["User"]["SessionID"])).Value();
				authUser.SessionKey = ((json::String)(configDocument["User"]["SessionKey"])).Value();
				authUser.Username = ((json::String)(configDocument["User"]["Username"])).Value();

				std::string userElevation = ((json::String)(configDocument["User"]["Elevation"])).Value();
				if(userElevation == "Admin")
					authUser.UserElevation = User::ElevationAdmin;
				else if(userElevation == "Mod")
					authUser.UserElevation = User::ElevationModerator;
				else
					authUser.UserElevation = User::ElevationNone;
			}
			catch (json::Exception &e)
			{
				authUser = User(0, "");
				std::cerr << "Error: Could not read data from prefs: " << e.what() << std::endl;
			}
		}
#ifndef MACOSX
		configFile.close();
#endif
	}
}

void Client::Initialise(std::string proxyString)
{

	if(GetPrefBool("version.update", false)==true)
	{
		SetPref("version.update", false);
		update_finish();
	}

	if(proxyString.length())
		http_init((char*)proxyString.c_str());
	else
		http_init(NULL);

	//Read stamps library
	std::ifstream stampsLib;
	stampsLib.open(STAMPS_DIR PATH_SEP "stamps.def", std::ios::binary);
	while(!stampsLib.eof())
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
	versionCheckRequest = http_async_req_start(NULL, "http://" SERVER "/Startup.json", NULL, 0, 0);

	if(authUser.ID)
	{
		std::string idTempString = format::NumberToString<int>(authUser.ID);
		char *id = new char[idTempString.length() + 1];
		std::strcpy (id, idTempString.c_str());
		char *session = new char[authUser.SessionID.length() + 1];
		std::strcpy (session, authUser.SessionID.c_str());
		http_auth_headers(versionCheckRequest, id, NULL, session);
		delete[] id;
		delete[] session;
	}
}

bool Client::DoInstallation()
{
#if defined(WIN)
	int returnval;
	LONG rresult;
	HKEY newkey;
	char *currentfilename = exe_name();
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

	if(iconname) free(iconname);
	if(opencommand) free(opencommand);
	if(protocolcommand) free(protocolcommand);
	if(currentfilename) free(currentfilename);
	
	return returnval;
#elif defined(LIN)
	#include "icondoc.h"

	char *currentfilename = exe_name();
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
"NoDisplay=true\n";
	char *protocolfiledata = (char *)malloc(strlen(protocolfiledata_tmp)+strlen(currentfilename)+100);
	strcpy(protocolfiledata, protocolfiledata_tmp);
	strappend(protocolfiledata, "Exec=");
	strappend(protocolfiledata, currentfilename);
	strappend(protocolfiledata, " ptsave %u\n");
	f = fopen("powdertoy-tpt-ptsave.desktop", "wb");
	if (!f)
		return 0;
	fwrite(protocolfiledata, 1, strlen(protocolfiledata), f);
	fclose(f);
	system("xdg-desktop-menu install powdertoy-tpt-ptsave.desktop");

	const char *desktopfiledata_tmp =
"[Desktop Entry]\n"
"Type=Application\n"
"Name=Powder Toy\n"
"Comment=Physics sandbox game\n"
"MimeType=application/vnd.powdertoy.save;\n"
"NoDisplay=true\n";
	char *desktopfiledata = (char *)malloc(strlen(desktopfiledata_tmp)+strlen(currentfilename)+100);
	strcpy(desktopfiledata, desktopfiledata_tmp);
	strappend(desktopfiledata, "Exec=");
	strappend(desktopfiledata, currentfilename);
	strappend(desktopfiledata, " open %f\n");
	f = fopen("powdertoy-tpt.desktop", "wb");
	if (!f)
		return 0;
	fwrite(desktopfiledata, 1, strlen(desktopfiledata), f);
	fclose(f);
	system("xdg-mime install powdertoy-save.xml");
	system("xdg-desktop-menu install powdertoy-tpt.desktop");
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
	system("xdg-icon-resource install --noupdate --context mimetypes --size 32 powdertoy-save-32.png application-vnd.powdertoy.save");
	system("xdg-icon-resource install --noupdate --context mimetypes --size 16 powdertoy-save-16.png application-vnd.powdertoy.save");
	system("xdg-icon-resource forceupdate");
	system("xdg-mime default powdertoy-tpt.desktop application/vnd.powdertoy.save");
	system("xdg-mime default powdertoy-tpt-ptsave.desktop x-scheme-handler/ptsave");
	unlink("powdertoy-save-32.png");
	unlink("powdertoy-save-16.png");
	unlink("powdertoy-save.xml");
	unlink("powdertoy-tpt.desktop");
	unlink("powdertoy-tpt-ptsave.desktop");
	return true;
#elif defined MACOSX
	return false;
#endif
}

void Client::SetProxy(std::string proxy)
{
	http_done();
	if(proxy.length())
		http_init((char*)proxy.c_str());
	else
		http_init(NULL);
}

std::vector<std::string> Client::DirectorySearch(std::string directory, std::string search, std::string extension)
{
	std::vector<std::string> extensions;
	extensions.push_back(extension);
	for (std::string::iterator iter = search.begin(); iter != search.end(); ++iter)
		*iter = toupper(*iter);
	return DirectorySearch(directory, search, extensions);
}

std::vector<std::string> Client::DirectorySearch(std::string directory, std::string search, std::vector<std::string> extensions)
{
	//Get full file listing
	//Normalise directory string, ensure / or \ is present
	if(*directory.rbegin() != '/' && *directory.rbegin() != '\\')
		directory += PATH_SEP;
	std::vector<std::string> directoryList;
#if defined(WIN) && !defined(__GNUC__)
	//Windows
	struct _finddata_t currentFile;
	intptr_t findFileHandle;
	std::string fileMatch = directory + "*.*";
	findFileHandle = _findfirst(fileMatch.c_str(), &currentFile);
	if (findFileHandle == -1L)
	{
#ifdef DEBUG
		printf("Unable to open directory: %s\n", directory.c_str());
#endif
		return std::vector<std::string>();
	}
	do
	{
		std::string currentFileName = std::string(currentFile.name);
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
		return std::vector<std::string>();
	}
	while(directoryEntry = readdir(directoryHandle))
	{
		std::string currentFileName = std::string(directoryEntry->d_name);
		if(currentFileName.length()>4)
			directoryList.push_back(directory+currentFileName);
	}
	closedir(directoryHandle);
#endif

	std::vector<std::string> searchResults;
	for(std::vector<std::string>::iterator iter = directoryList.begin(), end = directoryList.end(); iter != end; ++iter)
	{
		std::string filename = *iter, tempfilename = *iter;
		bool extensionMatch = !extensions.size();
		for(std::vector<std::string>::iterator extIter = extensions.begin(), extEnd = extensions.end(); extIter != extEnd; ++extIter)
		{
			int filenameLength = filename.length()-(*extIter).length();
			if(filename.find(*extIter, filenameLength) == filenameLength)
			{
				extensionMatch = true;
				tempfilename = filename.substr(0, filenameLength);
				break;
			}
		}
		for (std::string::iterator iter = tempfilename.begin(); iter != tempfilename.end(); ++iter)
			*iter = toupper(*iter);
		bool searchMatch = !search.size();
		if(search.size() && tempfilename.find(search)!=std::string::npos)
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

bool Client::WriteFile(std::vector<unsigned char> fileData, std::string filename)
{
	bool saveError = false;
	try
	{
		std::ofstream fileStream;
		fileStream.open(std::string(filename).c_str(), std::ios::binary);
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

bool Client::FileExists(std::string filename)
{
	bool exists = false;
	try
	{
		std::ifstream fileStream;
		fileStream.open(std::string(filename).c_str(), std::ios::binary);
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

bool Client::WriteFile(std::vector<char> fileData, std::string filename)
{
	bool saveError = false;
	try
	{
		std::ofstream fileStream;
		fileStream.open(std::string(filename).c_str(), std::ios::binary);
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

std::vector<unsigned char> Client::ReadFile(std::string filename)
{
	try
	{
		std::ifstream fileStream;
		fileStream.open(std::string(filename).c_str(), std::ios::binary);
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

void Client::SetMessageOfTheDay(std::string message)
{
	messageOfTheDay = message;
	notifyMessageOfTheDay();
}

std::string Client::GetMessageOfTheDay()
{
	return messageOfTheDay;
}

void Client::AddServerNotification(std::pair<std::string, std::string> notification)
{
	serverNotifications.push_back(notification);
	notifyNewNotification(notification);
}

std::vector<std::pair<std::string, std::string> > Client::GetServerNotifications()
{
	return serverNotifications;
}

void Client::Tick()
{
	//Check thumbnail queue
	RequestBroker::Ref().FlushThumbQueue();

	//Check status on version check request
	if(versionCheckRequest && http_async_req_status(versionCheckRequest))
	{
		int status;
		int dataLength;
		char * data = http_async_req_stop(versionCheckRequest, &status, &dataLength);
		versionCheckRequest = NULL;

		if(status != 200)
		{
			if(data)
				free(data);
		}
		else if(data)
		{
			std::istringstream dataStream(data);

			try
			{
				json::Object objDocument;
				json::Reader::Read(objDocument, dataStream);

				//Check session
				json::Boolean sessionStatus = objDocument["Session"];
				if(!sessionStatus.Value())
				{
					SetAuthUser(User(0, ""));
				}

				//Notifications from server
				json::Array notificationsArray = objDocument["Notifications"];
				for(int j = 0; j < notificationsArray.Size(); j++)
				{
					json::String notificationLink = notificationsArray[j]["Link"];
					json::String notificationText = notificationsArray[j]["Text"];

					std::pair<std::string, std::string> item = std::pair<std::string, std::string>(notificationText.Value(), notificationLink.Value());
					AddServerNotification(item);
				}


				//MOTD
				json::String messageOfTheDay = objDocument["MessageOfTheDay"];
				this->messageOfTheDay = messageOfTheDay.Value();
				notifyMessageOfTheDay();

#ifndef IGNORE_UPDATES
				//Check for updates
				json::Object versions = objDocument["Updates"];
				
				json::Object stableVersion = versions["Stable"];
				json::Object betaVersion = versions["Beta"];
				json::Object snapshotVersion = versions["Snapshot"];

				json::Number stableMajor = stableVersion["Major"];
				json::Number stableMinor = stableVersion["Minor"];
				json::Number stableBuild = stableVersion["Build"];
				json::String stableFile = stableVersion["File"];

				json::Number betaMajor = betaVersion["Major"];
				json::Number betaMinor = betaVersion["Minor"];
				json::Number betaBuild = betaVersion["Build"];
				json::String betaFile = betaVersion["File"];

				json::Number snapshotSnapshot = snapshotVersion["Snapshot"];
				json::String snapshotFile = snapshotVersion["File"];

				if(stableMajor.Value()>SAVE_VERSION || (stableMinor.Value()>MINOR_VERSION && stableMajor.Value()==SAVE_VERSION) || stableBuild.Value()>BUILD_NUM)
				{
					updateAvailable = true;
					updateInfo = UpdateInfo(stableMajor.Value(), stableMinor.Value(), stableBuild.Value(), stableFile.Value(), UpdateInfo::Stable);
				}

#ifdef BETA
				if(betaMajor.Value()>SAVE_VERSION || (betaMinor.Value()>MINOR_VERSION && betaMajor.Value()==SAVE_VERSION) || betaBuild.Value()>BUILD_NUM)
				{
					updateAvailable = true;
					updateInfo = UpdateInfo(betaMajor.Value(), betaMinor.Value(), betaBuild.Value(), betaFile.Value(), UpdateInfo::Beta);
				}
#endif

#ifdef SNAPSHOT
				if(snapshotSnapshot.Value() > SNAPSHOT_ID)
				{
					updateAvailable = true;
					updateInfo = UpdateInfo(snapshotSnapshot.Value(), snapshotFile.Value(), UpdateInfo::Snapshot);
				}
#endif

				if(updateAvailable)
				{
					notifyUpdateAvailable();
				}
#endif
			}
			catch (json::Exception &e)
			{
				//Do nothing
			}

			if(data)
				free(data);
		}
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

void Client::notifyNewNotification(std::pair<std::string, std::string> notification)
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
#ifdef MACOSX
	std::stringstream configFile;
#else
	std::ofstream configFile;
	configFile.open("powder.pref", std::ios::trunc);
#endif
	
	if(configFile)
	{
		if(authUser.ID)
		{
			configDocument["User"]["ID"] = json::Number(authUser.ID);
			configDocument["User"]["SessionID"] = json::String(authUser.SessionID);
			configDocument["User"]["SessionKey"] = json::String(authUser.SessionKey);
			configDocument["User"]["Username"] = json::String(authUser.Username);
			if(authUser.UserElevation == User::ElevationAdmin)
				configDocument["User"]["Elevation"] = json::String("Admin");
			else if(authUser.UserElevation == User::ElevationModerator)
				configDocument["User"]["Elevation"] = json::String("Mod");
			else
				configDocument["User"]["Elevation"] = json::String("None");
		}
		else
		{
			configDocument["User"] = json::Null();
		}
		json::Writer::Write(configDocument, configFile);

#ifdef MACOSX
		std::string prefString = configFile.str();

		char prefData[prefString.length()+1];
		std::strcpy(prefData, prefString.c_str());
		writeUserPreferences(prefData);
#else
		configFile.close();
#endif
	}
}

void Client::Shutdown()
{
	RequestBroker::Ref().Shutdown();
	ClearThumbnailRequests();
	http_done();

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
	int gameDataLength;
	char * gameData = NULL;
	int dataStatus;
	char * data;
	int dataLength = 0;
	std::stringstream userIDStream;
	userIDStream << authUser.ID;
	if(authUser.ID)
	{
		if(!save.GetGameSave())
		{
			lastError = "Empty game save";
			return RequestFailure;
		}
		save.SetID(0);

		gameData = save.GetGameSave()->Serialise(gameDataLength);

		if(!gameData)
		{
			lastError = "Cannot upload game save";
			return RequestFailure;
		}

		char *saveName = new char[save.GetName().length() + 1];
		std::strcpy (saveName, save.GetName().c_str());
		char *saveDescription = new char[save.GetDescription().length() + 1];
		std::strcpy (saveDescription, save.GetDescription().c_str());
		char *userid = new char[userIDStream.str().length() + 1];
		std::strcpy (userid, userIDStream.str().c_str());
		char *session = new char[authUser.SessionID.length() + 1];
		std::strcpy (session, authUser.SessionID.c_str());

		const char *const postNames[] = { "Name", "Description", "Data:save.bin", "Publish", NULL };
		const char *const postDatas[] = { saveName, saveDescription, gameData, (char *)(save.GetPublished()?"Public":"Private") };
		int postLengths[] = { save.GetName().length(), save.GetDescription().length(), gameDataLength, save.GetPublished()?6:7 };
		//std::cout << postNames[0] << " " << postDatas[0] << " " << postLengths[0] << std::endl;
		data = http_multipart_post("http://" SERVER "/Save.api", postNames, postDatas, postLengths, userid, NULL, session, &dataStatus, &dataLength);

		delete[] saveDescription;
		delete[] saveName;
		delete[] userid;
		delete[] session;
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(data && dataStatus == 200)
	{
		if(strncmp((const char *)data, "OK", 2)!=0)
		{
			if(gameData) delete[] gameData;
			lastError = std::string((const char *)data);
			free(data);
			return RequestFailure;
		}
		else
		{
			int tempID;
			std::stringstream saveIDStream((char *)(data+3));
			saveIDStream >> tempID;
			if(!tempID)
			{
				lastError = "Server did not return Save ID";
				return RequestFailure;
			}
			else
			{
				save.SetID(tempID);
			}
		}
		free(data);
		if(gameData) delete[] gameData;
		return RequestOkay;
	}
	else if(data)
	{
		free(data);
	}
	if(gameData) delete[] gameData;
	return RequestFailure;
}

void Client::MoveStampToFront(std::string stampID)
{
	for (std::list<std::string>::iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator)
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

SaveFile * Client::GetStamp(std::string stampID)
{
	std::string stampFile = std::string(STAMPS_DIR PATH_SEP + stampID + ".stm");
	if (!FileExists(stampFile))
		stampFile = stampID;
	if(FileExists(stampFile))
	{
		SaveFile * file = new SaveFile(stampID);
		try
		{
			GameSave * tempSave = new GameSave(ReadFile(stampFile));
			file->SetGameSave(tempSave);
		}
		catch (ParseException & e)
		{
			std::cerr << "Client: Invalid stamp file, " << stampID << " " << std::string(e.what()) << std::endl;
		}
		return file;
	}
	else
	{
		return NULL;
	}
}

void Client::DeleteStamp(std::string stampID)
{
	for (std::list<std::string>::iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator)
	{
		if((*iterator) == stampID)
		{
			std::stringstream stampFilename;
			stampFilename << STAMPS_DIR;
			stampFilename << PATH_SEP;
			stampFilename << stampID;
			stampFilename << ".stm";
			remove(stampFilename.str().c_str());
			stampIDs.erase(iterator);
			return;
		}
	}

	updateStamps();
}

std::string Client::AddStamp(GameSave * saveData)
{
	unsigned t=(unsigned)time(NULL);
	if (lastStampTime!=t)
	{
		lastStampTime=t;
		lastStampName=0;
	}
	else
		lastStampName++;
	std::stringstream saveID;
	//sprintf(saveID, "%08x%02x", lastStampTime, lastStampName);
	saveID
	<< std::setw(8) << std::setfill('0') << std::hex << lastStampTime
	<< std::setw(2) << std::setfill('0') << std::hex << lastStampName;

	MakeDirectory(STAMPS_DIR);

	int gameDataLength;
	char * gameData = saveData->Serialise(gameDataLength);

	std::ofstream stampStream;
	stampStream.open(std::string(STAMPS_DIR PATH_SEP + saveID.str()+".stm").c_str(), std::ios::binary);
	stampStream.write((const char *)gameData, gameDataLength);
	stampStream.close();

	delete[] gameData;

	stampIDs.push_front(saveID.str());

	updateStamps();

	return saveID.str();
}

void Client::updateStamps()
{
	MakeDirectory(STAMPS_DIR);

	std::ofstream stampsStream;
	stampsStream.open(std::string(STAMPS_DIR PATH_SEP "stamps.def").c_str(), std::ios::binary);
	for (std::list<std::string>::const_iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator)
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
		while (entry = readdir(directory))
		{
			if(strncmp(entry->d_name, "..", 3) && strncmp(entry->d_name, ".", 2) && strstr(entry->d_name, ".stm") && strlen(entry->d_name) == 14)
			{
				char stampname[11];
				strncpy(stampname, entry->d_name, 10);
				stampIDs.push_front(stampname);
			}
		}
		closedir(directory);
		updateStamps();
	}
}

int Client::GetStampsCount()
{
	return stampIDs.size();
}

std::vector<std::string> Client::GetStamps(int start, int count)
{
	if(start+count > stampIDs.size()) {
		if(start > stampIDs.size())
			return std::vector<std::string>();
		count = stampIDs.size()-start;
	}

	std::vector<std::string> stampRange;
	int index = 0;
	for (std::list<std::string>::const_iterator iterator = stampIDs.begin(), end = stampIDs.end(); iterator != end; ++iterator, ++index) {
		if(index>=start && index < start+count)
			stampRange.push_back(*iterator);
	}
	return stampRange;
}

RequestStatus Client::ExecVote(int saveID, int direction)
{
	lastError = "";
	int dataStatus;
	char * data;
	int dataLength = 0;
	std::stringstream idStream;
	idStream << saveID;

	if(authUser.ID)
	{
		char * directionText = (char*)(direction==1?"Up":"Down");
		std::string saveIDText = format::NumberToString<int>(saveID);
		std::string userIDText = format::NumberToString<int>(authUser.ID);

		char *id = new char[saveIDText.length() + 1];
		std::strcpy (id, saveIDText.c_str());
		char *userid = new char[userIDText.length() + 1];
		std::strcpy (userid, userIDText.c_str());
		char *session = new char[authUser.SessionID.length() + 1];
		std::strcpy (session, authUser.SessionID.c_str());

		const char *const postNames[] = { "ID", "Action", NULL };
		const char *const postDatas[] = { id, directionText };
		int postLengths[] = { saveIDText.length(), strlen(directionText) };
		//std::cout << postNames[0] << " " << postDatas[0] << " " << postLengths[0] << std::endl;
		data = http_multipart_post("http://" SERVER "/Vote.api", postNames, postDatas, postLengths, userid, NULL, session, &dataStatus, &dataLength);

		delete[] id;
		delete[] userid;
		delete[] session;
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(data && dataStatus == 200)
	{
		if(strncmp((const char *)data, "OK", 2)!=0)
		{
			lastError = std::string((const char *)data);
			free(data);
			return RequestFailure;
		}
		free(data);
		return RequestOkay;
	}
	else if(data)
	{
		free(data);
	}
	lastError = http_ret_text(dataStatus);
	return RequestFailure;
}

unsigned char * Client::GetSaveData(int saveID, int saveDate, int & dataLength)
{
	lastError = "";
	int dataStatus;
	unsigned char * data;
	dataLength = 0;
	std::stringstream urlStream;
	if(saveDate)
	{
		urlStream << "http://" << STATICSERVER << "/" << saveID << "_" << saveDate << ".cps";
	}
	else
	{
		urlStream << "http://" << STATICSERVER << "/" << saveID << ".cps";
	}

	char *url = new char[urlStream.str().length() + 1];
	std::strcpy (url, urlStream.str().c_str());
	data = (unsigned char *)http_simple_get(url, &dataStatus, &dataLength);
	delete[] url;

	if(data && dataStatus == 200)
	{
		return data;
	}
	else if(data)
	{
		free(data);
	}
	return NULL;
}

std::vector<unsigned char> Client::GetSaveData(int saveID, int saveDate)
{
	int dataSize;
	unsigned char * data = GetSaveData(saveID, saveDate, dataSize);

	std::vector<unsigned char> saveData(data, data+dataSize);

	delete[] data;
	return saveData;
}

RequestBroker::Request * Client::GetSaveDataAsync(int saveID, int saveDate)
{
	std::stringstream urlStream;
	if(saveDate){
		urlStream << "http://" << STATICSERVER << "/" << saveID << "_" << saveDate << ".cps";
	} else {
		urlStream << "http://" << STATICSERVER << "/" << saveID << ".cps";
	}
	return new WebRequest(urlStream.str());	
}

RequestBroker::Request * Client::SaveUserInfoAsync(UserInfo info)
{
	class StatusParser: public APIResultParser
	{
		virtual void * ProcessResponse(unsigned char * data, int dataLength)
		{
			try
			{
				std::istringstream dataStream((char*)data);
				json::Object objDocument;
				json::Reader::Read(objDocument, dataStream);
				json::Number tempStatus = objDocument["Status"];

				return (void*)(tempStatus.Value() == 1);
			}
			catch (json::Exception &e)
			{
				return 0;
			}
		}
		virtual void Cleanup(void * objectPtr)
		{
			//delete (UserInfo*)objectPtr;
		}
		virtual ~StatusParser() { }
	};
	std::map<std::string, std::string> postData;
	postData.insert(std::pair<std::string, std::string>("Location", info.Location));
	postData.insert(std::pair<std::string, std::string>("Biography", info.Biography));
	return new APIRequest("http://" SERVER "/Profile.json", postData, new StatusParser());	
}

RequestBroker::Request * Client::GetUserInfoAsync(std::string username)
{
	class UserInfoParser: public APIResultParser
	{
		virtual void * ProcessResponse(unsigned char * data, int dataLength)
		{
			try
			{
				std::istringstream dataStream((char*)data);
				json::Object objDocument;
				json::Reader::Read(objDocument, dataStream);
				json::Object tempUser = objDocument["User"];

				json::Number userIDTemp = tempUser["ID"];
				json::String usernameTemp = tempUser["Username"];
				json::String bioTemp = tempUser["Biography"];
				json::String locationTemp = tempUser["Location"];
				json::Number ageTemp = tempUser["Age"];
				
				return new UserInfo(
					userIDTemp.Value(),
					ageTemp.Value(),
					usernameTemp.Value(),
					bioTemp.Value(),
					locationTemp.Value());
			}
			catch (json::Exception &e)
			{
				return 0;
			}
		}
		virtual void Cleanup(void * objectPtr)
		{
			delete (UserInfo*)objectPtr;
		}
		virtual ~UserInfoParser() { }
	};
	return new APIRequest("http://" SERVER "/User.json?Name=" + username, new UserInfoParser());
}

LoginStatus Client::Login(std::string username, std::string password, User & user)
{
	lastError = "";
	std::stringstream hashStream;
	char passwordHash[33];
	char totalHash[33];

	user.ID = 0;
	user.Username = "";
	user.SessionID = "";
	user.SessionKey = "";

	//Doop
	md5_ascii(passwordHash, (const unsigned char *)password.c_str(), password.length());
	passwordHash[32] = 0;
	hashStream << username << "-" << passwordHash;
	md5_ascii(totalHash, (const unsigned char *)(hashStream.str().c_str()), hashStream.str().length());
	totalHash[32] = 0;

	char * data;
	int dataStatus, dataLength;
	const char *const postNames[] = { "Username", "Hash", NULL };
	const char *const postDatas[] = { (char*)username.c_str(), totalHash };
	int postLengths[] = { username.length(), 32 };
	data = http_multipart_post("http://" SERVER "/Login.json", postNames, postDatas, postLengths, NULL, NULL, NULL, &dataStatus, &dataLength);
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);
			json::Number tempStatus = objDocument["Status"];

			free(data);
			if(tempStatus.Value() == 1)
			{
				json::Number userIDTemp = objDocument["UserID"];
				json::String sessionIDTemp = objDocument["SessionID"];
				json::String sessionKeyTemp = objDocument["SessionKey"];
				json::String userElevationTemp = objDocument["Elevation"];
				
				json::Array notificationsArray = objDocument["Notifications"];
				for(int j = 0; j < notificationsArray.Size(); j++)
				{
					json::String notificationLink = notificationsArray[j]["Link"];
					json::String notificationText = notificationsArray[j]["Text"];

					std::pair<std::string, std::string> item = std::pair<std::string, std::string>(notificationText.Value(), notificationLink.Value());
					AddServerNotification(item);
				}

				user.Username = username;
				user.ID = userIDTemp.Value();
				user.SessionID = sessionIDTemp.Value();
				user.SessionKey = sessionKeyTemp.Value();
				std::string userElevation = userElevationTemp.Value();
				if(userElevation == "Admin")
					user.UserElevation = User::ElevationAdmin;
				else if(userElevation == "Mod")
					user.UserElevation = User::ElevationModerator;
				else
					user.UserElevation= User::ElevationNone;
				return LoginOkay;
			}
			else
			{
				json::String tempError = objDocument["Error"];
				lastError = tempError.Value();
				return LoginError;
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			return LoginError;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return LoginError;
}

RequestStatus Client::DeleteSave(int saveID)
{
	lastError = "";
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Delete.json?ID=" << saveID << "&Mode=Delete&Key=" << authUser.SessionKey;
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			int status = ((json::Number)objDocument["Status"]).Value();

			if(status!=1)
				goto failure;
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			goto failure;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
		goto failure;
	}
	if(data)
		free(data);
	return RequestOkay;
failure:
	if(data)
		free(data);
	return RequestFailure;
}

RequestStatus Client::AddComment(int saveID, std::string comment)
{
	lastError = "";
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Comments.json?ID=" << saveID;
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		
		const char *const postNames[] = { "Comment", NULL };
		const char *const postDatas[] = { (char*)(comment.c_str()) };
		int postLengths[] = { comment.length() };
		data = http_multipart_post((char *)urlStream.str().c_str(), postNames, postDatas, postLengths, (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			int status = ((json::Number)objDocument["Status"]).Value();

			if(status!=1)
			{
				lastError = ((json::String)objDocument["Error"]).Value();
			}

			if(status!=1)
				goto failure;
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			goto failure;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
		goto failure;
	}
	if(data)
		free(data);
	return RequestOkay;
failure:
	if(data)
		free(data);
	return RequestFailure;
}

RequestStatus Client::FavouriteSave(int saveID, bool favourite)
{
	lastError = "";
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Favourite.json?ID=" << saveID << "&Key=" << authUser.SessionKey;
	if(!favourite)
		urlStream << "&Mode=Remove";
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			int status = ((json::Number)objDocument["Status"]).Value();

			if(status!=1)
			{
				lastError = ((json::String)objDocument["Error"]).Value();
				goto failure;
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			goto failure;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
		goto failure;
	}
	if(data)
		free(data);
	return RequestOkay;
failure:
	if(data)
		free(data);
	return RequestFailure;
}

RequestStatus Client::ReportSave(int saveID, std::string message)
{
	lastError = "";
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Report.json?ID=" << saveID << "&Key=" << authUser.SessionKey;
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;

		const char *const postNames[] = { "Reason", NULL };
		const char *const postDatas[] = { (char*)(message.c_str()) };
		int postLengths[] = { message.length() };
		data = http_multipart_post((char *)urlStream.str().c_str(), postNames, postDatas, postLengths, (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			int status = ((json::Number)objDocument["Status"]).Value();

			if(status!=1)
			{
				lastError = ((json::String)objDocument["Error"]).Value();
				goto failure;
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			goto failure;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
		goto failure;
	}
	if(data)
		free(data);
	return RequestOkay;
failure:
	if(data)
		free(data);
	return RequestFailure;
}

RequestStatus Client::UnpublishSave(int saveID)
{
	lastError = "";
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Delete.json?ID=" << saveID << "&Mode=Unpublish&Key=" << authUser.SessionKey;
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			int status = ((json::Number)objDocument["Status"]).Value();

			if(status!=1)
			{
				lastError = ((json::String)objDocument["Error"]).Value();
				goto failure;
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			goto failure;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
		goto failure;
	}
	if(data)
		free(data);
	return RequestOkay;
failure:
	if(data)
		free(data);
	return RequestFailure;
}

SaveInfo * Client::GetSave(int saveID, int saveDate)
{
	lastError = "";
	std::stringstream urlStream;
	urlStream << "http://" << SERVER  << "/Browse/View.json?ID=" << saveID;
	if(saveDate)
	{
		urlStream << "&Date=" << saveDate;
	}
	char * data;
	int dataStatus, dataLength;
	//Save(int _id, int _votesUp, int _votesDown, string _userName, string _name, string description_, string date_, bool published_):
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			json::Number tempID = objDocument["ID"];
			json::Number tempScoreUp = objDocument["ScoreUp"];
			json::Number tempScoreDown = objDocument["ScoreDown"];
			json::Number tempMyScore = objDocument["ScoreMine"];
			json::String tempUsername = objDocument["Username"];
			json::String tempName = objDocument["Name"];
			json::String tempDescription = objDocument["Description"];
			json::Number tempDate = objDocument["Date"];
			json::Boolean tempPublished = objDocument["Published"];
			json::Boolean tempFavourite = objDocument["Favourite"];
			json::Number tempComments = objDocument["Comments"];
			json::Number tempViews = objDocument["Views"];
			json::Number tempVersion = objDocument["Version"];

			json::Array tagsArray = objDocument["Tags"];
			std::list<std::string> tempTags;

			for(int j = 0; j < tagsArray.Size(); j++)
			{
				json::String tempTag = tagsArray[j];
				tempTags.push_back(tempTag.Value());
			}

			SaveInfo * tempSave = new SaveInfo(
					tempID.Value(),
					tempDate.Value(),
					tempScoreUp.Value(),
					tempScoreDown.Value(),
					tempMyScore.Value(),
					tempUsername.Value(),
					tempName.Value(),
					tempDescription.Value(),
					tempPublished.Value(),
					tempTags
					);
			tempSave->Comments = tempComments.Value();
			tempSave->Favourite = tempFavourite.Value();
			tempSave->Views = tempViews.Value();
			tempSave->Version = tempVersion.Value();
			free(data);
			return tempSave;
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			free(data);
			return NULL;
		}
	}
	else
	{
		if (data) free(data);
		lastError = http_ret_text(dataStatus);
	}
	return NULL;
}

RequestBroker::Request * Client::GetSaveAsync(int saveID, int saveDate)
{
	std::stringstream urlStream;
	urlStream << "http://" << SERVER  << "/Browse/View.json?ID=" << saveID;
	if(saveDate)
	{
		urlStream << "&Date=" << saveDate;
	}

	class SaveInfoParser: public APIResultParser
	{
		virtual void * ProcessResponse(unsigned char * data, int dataLength)
		{
			try
			{
				std::istringstream dataStream((char*)data);
				json::Object objDocument;
				json::Reader::Read(objDocument, dataStream);

				json::Number tempID = objDocument["ID"];
				json::Number tempScoreUp = objDocument["ScoreUp"];
				json::Number tempScoreDown = objDocument["ScoreDown"];
				json::Number tempMyScore = objDocument["ScoreMine"];
				json::String tempUsername = objDocument["Username"];
				json::String tempName = objDocument["Name"];
				json::String tempDescription = objDocument["Description"];
				json::Number tempDate = objDocument["Date"];
				json::Boolean tempPublished = objDocument["Published"];
				json::Boolean tempFavourite = objDocument["Favourite"];
				json::Number tempComments = objDocument["Comments"];
				json::Number tempViews = objDocument["Views"];
				json::Number tempVersion = objDocument["Version"];

				json::Array tagsArray = objDocument["Tags"];
				std::list<std::string> tempTags;

				for(int j = 0; j < tagsArray.Size(); j++)
				{
					json::String tempTag = tagsArray[j];
					tempTags.push_back(tempTag.Value());
				}

				SaveInfo * tempSave = new SaveInfo(
						tempID.Value(),
						tempDate.Value(),
						tempScoreUp.Value(),
						tempScoreDown.Value(),
						tempMyScore.Value(),
						tempUsername.Value(),
						tempName.Value(),
						tempDescription.Value(),
						tempPublished.Value(),
						tempTags
						);
				tempSave->Comments = tempComments.Value();
				tempSave->Favourite = tempFavourite.Value();
				tempSave->Views = tempViews.Value();
				tempSave->Version = tempVersion.Value();
				return tempSave;
			}
			catch (json::Exception &e)
			{
				return NULL;
			}
		}
		virtual void Cleanup(void * objectPtr)
		{
			delete (SaveInfo*)objectPtr;
		}
		virtual ~SaveInfoParser() { }
	};
	return new APIRequest(urlStream.str(), new SaveInfoParser());
}

Thumbnail * Client::GetPreview(int saveID, int saveDate)
{
	std::stringstream urlStream;
	urlStream << "http://" << STATICSERVER  << "/" << saveID;
	if(saveDate)
	{
		urlStream << "_" << saveDate;
	}
	urlStream << "_large.pti";
	pixel * thumbData;
	char * data;
	int status, data_size, imgw, imgh;
	data = http_simple_get((char *)urlStream.str().c_str(), &status, &data_size);
	if (status == 200 && data)
	{
		thumbData = Graphics::ptif_unpack(data, data_size, &imgw, &imgh);
		if(data)
		{
			free(data);
		}
		if(thumbData)
		{
			return new Thumbnail(saveID, saveDate, thumbData, ui::Point(imgw, imgh));
			free(thumbData);
		}
		else
		{
			thumbData = (pixel *)malloc((128*128) * PIXELSIZE);
			return new Thumbnail(saveID, saveDate, thumbData, ui::Point(128, 128));
			free(thumbData);
		}
	}
	else
	{
		if(data)
		{
			free(data);
		}
	}
	return new Thumbnail(saveID, saveDate, (pixel *)malloc((128*128) * PIXELSIZE), ui::Point(128, 128));
}

RequestBroker::Request * Client::GetCommentsAsync(int saveID, int start, int count)
{
	class CommentsParser: public APIResultParser
	{
		virtual void * ProcessResponse(unsigned char * data, int dataLength)
		{
			std::vector<SaveComment*> * commentArray = new std::vector<SaveComment*>();
			try
			{
				std::istringstream dataStream((char*)data);
				json::Array commentsArray;
				json::Reader::Read(commentsArray, dataStream);

				for(int j = 0; j < commentsArray.Size(); j++)
				{
					json::Number tempUserID = commentsArray[j]["UserID"];
					json::String tempUsername = commentsArray[j]["Username"];
					json::String tempFormattedUsername = commentsArray[j]["FormattedUsername"];
					json::String tempComment = commentsArray[j]["Text"];
					commentArray->push_back(
								new SaveComment(
									tempUserID.Value(),
									tempUsername.Value(),
									tempFormattedUsername.Value(),
									tempComment.Value()
									)
								);
				}
				return commentArray;
			}
			catch (json::Exception &e)
			{
				delete commentArray;
				return NULL;
			}
		}
		virtual void Cleanup(void * objectPtr)
		{
			delete (std::vector<SaveComment*>*)objectPtr;
		}
		virtual ~CommentsParser() { }
	};

	std::stringstream urlStream;
	urlStream << "http://" << SERVER << "/Browse/Comments.json?ID=" << saveID << "&Start=" << start << "&Count=" << count;
	return new APIRequest(urlStream.str(), new CommentsParser());
}

std::vector<SaveComment*> * Client::GetComments(int saveID, int start, int count)
{
	lastError = "";
	std::vector<SaveComment*> * commentArray = new std::vector<SaveComment*>();

	std::stringstream urlStream;
	char * data;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Comments.json?ID=" << saveID << "&Start=" << start << "&Count=" << count;
	data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Array commentsArray;
			json::Reader::Read(commentsArray, dataStream);

			for(int j = 0; j < commentsArray.Size(); j++)
			{
				json::Number tempUserID = commentsArray[j]["UserID"];
				json::String tempUsername = commentsArray[j]["Username"];
				json::String tempFormattedUsername = commentsArray[j]["FormattedUsername"];
				json::String tempComment = commentsArray[j]["Text"];
				commentArray->push_back(
							new SaveComment(
								tempUserID.Value(),
								tempUsername.Value(),
								tempFormattedUsername.Value(),
								tempComment.Value()
								)
							);
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return commentArray;
}

std::vector<std::pair<std::string, int> > * Client::GetTags(int start, int count, std::string query, int & resultCount)
{
	lastError = "";
	resultCount = 0;
	std::vector<std::pair<std::string, int> > * tagArray = new std::vector<std::pair<std::string, int> >();
	std::stringstream urlStream;
	char * data;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/Tags.json?Start=" << start << "&Count=" << count;
	if(query.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << URLEscape(query);
	}
	
	data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			json::Number tempCount = objDocument["TagTotal"];
			resultCount = tempCount.Value();
			json::Array tagsArray = objDocument["Tags"];
			for(int j = 0; j < tagsArray.Size(); j++)
			{
				json::Number tagCount = tagsArray[j]["Count"];
				json::String tag = tagsArray[j]["Tag"];
				tagArray->push_back(std::pair<std::string, int>(tag.Value(), (int)tagCount.Value()));
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return tagArray;
}

std::vector<SaveInfo*> * Client::SearchSaves(int start, int count, std::string query, std::string sort, std::string category, int & resultCount)
{
	lastError = "";
	resultCount = 0;
	std::vector<SaveInfo*> * saveArray = new std::vector<SaveInfo*>();
	std::stringstream urlStream;
	char * data;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse.json?Start=" << start << "&Count=" << count;
	if(query.length() || sort.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << URLEscape(query);
		if(sort == "date")
		{
			if(query.length())
				urlStream << URLEscape(" ");
			urlStream << URLEscape("sort:") << URLEscape(sort);
		}
	}
	if(category.length())
	{
		urlStream << "&Category=" << URLEscape(category);
	}
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			json::Number tempCount = objDocument["Count"];
			resultCount = tempCount.Value();
			json::Array savesArray = objDocument["Saves"];
			for(int j = 0; j < savesArray.Size(); j++)
			{
				json::Number tempID = savesArray[j]["ID"];
				json::Number tempDate = savesArray[j]["Date"];
				json::Number tempScoreUp = savesArray[j]["ScoreUp"];
				json::Number tempScoreDown = savesArray[j]["ScoreDown"];
				json::String tempUsername = savesArray[j]["Username"];
				json::String tempName = savesArray[j]["Name"];
				json::Number tempVersion = savesArray[j]["Version"];
				json::Boolean tempPublished = savesArray[j]["Published"];
				SaveInfo * tempSaveInfo = new SaveInfo(
								tempID.Value(),
								tempDate.Value(),
								tempScoreUp.Value(),
								tempScoreDown.Value(),
								tempUsername.Value(),
								tempName.Value()
								);
				tempSaveInfo->Version = tempVersion.Value();
				tempSaveInfo->SetPublished(tempPublished);
				saveArray->push_back(tempSaveInfo);
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return saveArray;
}

void Client::ClearThumbnailRequests()
{
	for(int i = 0; i < IMGCONNS; i++)
	{
		if(activeThumbRequests[i])
		{
			http_async_req_close(activeThumbRequests[i]);
			activeThumbRequests[i] = NULL;
			activeThumbRequestTimes[i] = 0;
			activeThumbRequestCompleteTimes[i] = 0;
		}
	}
}

Thumbnail * Client::GetThumbnail(int saveID, int saveDate)
{
	std::stringstream urlStream;
	std::stringstream idStream;
	int i = 0, currentTime = time(NULL);
	//Check active requests for any "forgotten" requests
	for(i = 0; i < IMGCONNS; i++)
	{
		//If the request is active, and we've received a response
		if(activeThumbRequests[i] && http_async_req_status(activeThumbRequests[i]))
		{
			//If we haven't already, mark the request as completed
			if(!activeThumbRequestCompleteTimes[i])
			{
				activeThumbRequestCompleteTimes[i] = time(NULL);
			}
			else if(activeThumbRequestCompleteTimes[i] < (currentTime-2)) //Otherwise, if it completed more than 2 seconds ago, destroy it.
			{
				http_async_req_close(activeThumbRequests[i]);
				activeThumbRequests[i] = NULL;
				activeThumbRequestTimes[i] = 0;
				activeThumbRequestCompleteTimes[i] = 0;
			}
		}
	}
	for(i = 0; i < THUMB_CACHE_SIZE; i++)
	{
		if(thumbnailCache[i] && thumbnailCache[i]->ID == saveID && thumbnailCache[i]->Datestamp == saveDate)
			return thumbnailCache[i];
	}
	urlStream << "http://" << STATICSERVER  << "/" << saveID;
	if(saveDate)
	{
		urlStream << "_" << saveDate;
	}
	urlStream << "_small.pti";
	idStream << saveID << ":" << saveDate;
	std::string idString = idStream.str();
	bool found = false;
	for(i = 0; i < IMGCONNS; i++)
	{
		if(activeThumbRequests[i] && activeThumbRequestIDs[i] == idString)
		{
			found = true;
			if(http_async_req_status(activeThumbRequests[i]))
			{
				pixel * thumbData;
				char * data;
				int status, data_size, imgw, imgh;
				data = http_async_req_stop(activeThumbRequests[i], &status, &data_size);
				free(activeThumbRequests[i]);
				activeThumbRequests[i] = NULL;
				if (status == 200 && data)
				{
					thumbData = Graphics::ptif_unpack(data, data_size, &imgw, &imgh);
					if(data)
					{
						free(data);
					}
					thumbnailCacheNextID %= THUMB_CACHE_SIZE;
					if(thumbnailCache[thumbnailCacheNextID])
					{
						delete thumbnailCache[thumbnailCacheNextID];
					}
					if(thumbData)
					{
						thumbnailCache[thumbnailCacheNextID] = new Thumbnail(saveID, saveDate, thumbData, ui::Point(imgw, imgh));
						free(thumbData);
					}
					else
					{
						thumbData = (pixel *)malloc((128*128) * PIXELSIZE);
						thumbnailCache[thumbnailCacheNextID] = new Thumbnail(saveID, saveDate, thumbData, ui::Point(128, 128));
						free(thumbData);
					}
					return thumbnailCache[thumbnailCacheNextID++];
				}
				else
				{
					if(data)
					{
						free(data);
					}
					thumbnailCacheNextID %= THUMB_CACHE_SIZE;
					if(thumbnailCache[thumbnailCacheNextID])
					{
						delete thumbnailCache[thumbnailCacheNextID];
					}
					thumbData = (pixel *)malloc((128*128) * PIXELSIZE);
					thumbnailCache[thumbnailCacheNextID] = new Thumbnail(saveID, saveDate, thumbData, ui::Point(128, 128));
					free(thumbData);
					return thumbnailCache[thumbnailCacheNextID++];
				}
			}
		}
	}
	if(!found)
	{
		for(i = 0; i < IMGCONNS; i++)
		{
			if(!activeThumbRequests[i])
			{
				activeThumbRequests[i] = http_async_req_start(NULL, (char *)urlStream.str().c_str(), NULL, 0, 0);
				activeThumbRequestTimes[i] = currentTime;
				activeThumbRequestCompleteTimes[i] = 0;
				activeThumbRequestIDs[i] = idString;
				return NULL;
			}
		}
	}
	//http_async_req_start(http, urlStream.str().c_str(), NULL, 0, 1);
	return NULL;
}

std::list<std::string> * Client::RemoveTag(int saveID, std::string tag)
{
	lastError = "";
	std::list<std::string> * tags = NULL;
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/EditTag.json?Op=delete&ID=" << saveID << "&Tag=" << tag << "&Key=" << authUser.SessionKey;;
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return NULL;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object responseObject;
			json::Reader::Read(responseObject, dataStream);

			json::Number status = responseObject["Status"];

			if(status.Value()==0)
			{
				json::String error = responseObject["Error"];
				lastError = error.Value();
			}
			else
			{
				json::Array tagsArray = responseObject["Tags"];

				tags = new std::list<std::string>();

				for(int j = 0; j < tagsArray.Size(); j++)
				{
					json::String tempTag = tagsArray[j];
					tags->push_back(tempTag.Value());
				}
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return tags;
}

std::list<std::string> * Client::AddTag(int saveID, std::string tag)
{
	lastError = "";
	std::list<std::string> * tags = NULL;
	std::stringstream urlStream;
	char * data = NULL;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/EditTag.json?Op=add&ID=" << saveID << "&Tag=" << tag << "&Key=" << authUser.SessionKey;
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return NULL;
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object responseObject;
			json::Reader::Read(responseObject, dataStream);

			json::Number status = responseObject["Status"];

			if(status.Value()==0)
			{
				json::String error = responseObject["Error"];
				lastError = error.Value();
			}
			else
			{
				json::Array tagsArray = responseObject["Tags"];

				tags = new std::list<std::string>();

				for(int j = 0; j < tagsArray.Size(); j++)
				{
					json::String tempTag = tagsArray[j];
					tags->push_back(tempTag.Value());
				}
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return tags;
}

std::vector<std::string> Client::explodePropertyString(std::string property)
{
	std::vector<std::string> stringArray;
	std::string current = "";
	for (std::string::iterator iter = property.begin(); iter != property.end(); ++iter) {
		if (*iter == '.') {
			if (current.length() > 0) {
				stringArray.push_back(current);
				current = "";
			}
		} else {
			current += *iter;
		}
	}
	if(current.length() > 0)
		stringArray.push_back(current);
	return stringArray;
}

std::string Client::GetPrefString(std::string property, std::string defaultValue)
{
	try
	{
		json::String value = GetPref(property);
		return value.Value();
	}
	catch (json::Exception & e)
	{

	}
	return defaultValue;
}

double Client::GetPrefNumber(std::string property, double defaultValue)
{
	try
	{
		json::Number value = GetPref(property);
		return value.Value();
	}
	catch (json::Exception & e)
	{

	}
	return defaultValue;
}

int Client::GetPrefInteger(std::string property, int defaultValue)
{
	try
	{
		std::stringstream defHexInt;
		defHexInt << std::hex << defaultValue;

		std::string hexString = GetPrefString(property, defHexInt.str());
		int finalValue = defaultValue;

		std::stringstream hexInt;
		hexInt << hexString;

		hexInt >> std::hex >> finalValue;

		return finalValue;
	}
	catch (json::Exception & e)
	{

	}
	catch(std::exception & e)
	{

	}
	return defaultValue;
}

unsigned int Client::GetPrefUInteger(std::string property, unsigned int defaultValue)
{
	try
	{
		std::stringstream defHexInt;
		defHexInt << std::hex << defaultValue;

		std::string hexString = GetPrefString(property, defHexInt.str());
		unsigned int finalValue = defaultValue;

		std::stringstream hexInt;
		hexInt << hexString;

		hexInt >> std::hex >> finalValue;

		return finalValue;
	}
	catch (json::Exception & e)
	{

	}
	catch(std::exception & e)
	{

	}
	return defaultValue;
}

std::vector<std::string> Client::GetPrefStringArray(std::string property)
{
	try
	{
		json::Array value = GetPref(property);
		std::vector<std::string> strArray;
		for(json::Array::iterator iter = value.Begin(); iter != value.End(); ++iter)
		{
			try
			{
				json::String cValue = *iter;
				strArray.push_back(cValue.Value());
			}
			catch (json::Exception & e)
			{
				
			}
		}
		return strArray;
	}
	catch (json::Exception & e)
	{

	}
	return std::vector<std::string>();
}

std::vector<double> Client::GetPrefNumberArray(std::string property)
{
	try
	{
		json::Array value = GetPref(property);
		std::vector<double> strArray;
		for(json::Array::iterator iter = value.Begin(); iter != value.End(); ++iter)
		{
			try
			{
				json::Number cValue = *iter;
				strArray.push_back(cValue.Value());
			}
			catch (json::Exception & e)
			{
				
			}
		}
		return strArray;
	}
	catch (json::Exception & e)
	{

	}
	return std::vector<double>();
}

std::vector<int> Client::GetPrefIntegerArray(std::string property)
{
	try
	{
		json::Array value = GetPref(property);
		std::vector<int> intArray;
		for(json::Array::iterator iter = value.Begin(); iter != value.End(); ++iter)
		{
			try
			{
				json::String cValue = *iter;
				int finalValue = 0;
		
				std::string hexString = cValue.Value();
				std::stringstream hexInt;
				hexInt << std::hex << hexString;
				hexInt >> finalValue;

				intArray.push_back(finalValue);
			}
			catch (json::Exception & e)
			{

			}
		}
		return intArray;
	}
	catch (json::Exception & e)
	{

	}
	return std::vector<int>();
}

std::vector<unsigned int> Client::GetPrefUIntegerArray(std::string property)
{
	try
	{
		json::Array value = GetPref(property);
		std::vector<unsigned int> intArray;
		for(json::Array::iterator iter = value.Begin(); iter != value.End(); ++iter)
		{
			try
			{
				json::String cValue = *iter;
				unsigned int finalValue = 0;
		
				std::string hexString = cValue.Value();
				std::stringstream hexInt;
				hexInt << std::hex << hexString;
				hexInt >> finalValue;

				intArray.push_back(finalValue);
			}
			catch (json::Exception & e)
			{

			}
		}
		return intArray;
	}
	catch (json::Exception & e)
	{

	}
	return std::vector<unsigned int>();
}

std::vector<bool> Client::GetPrefBoolArray(std::string property)
{
	try
	{
		json::Array value = GetPref(property);
		std::vector<bool> strArray;
		for(json::Array::iterator iter = value.Begin(); iter != value.End(); ++iter)
		{
			try
			{
				json::Boolean cValue = *iter;
				strArray.push_back(cValue.Value());
			}
			catch (json::Exception & e)
			{
				
			}
		}
		return strArray;
	}
	catch (json::Exception & e)
	{

	}
	return std::vector<bool>();
}

bool Client::GetPrefBool(std::string property, bool defaultValue)
{
	try
	{
		json::Boolean value = GetPref(property);
		return value.Value();
	}
	catch (json::Exception & e)
	{

	}
	return defaultValue;
}

void Client::SetPref(std::string property, std::string value)
{
	json::UnknownElement stringValue = json::String(value);
	SetPref(property, stringValue);
}

void Client::SetPref(std::string property, double value)
{
	json::UnknownElement numberValue = json::Number(value);
	SetPref(property, numberValue);
}

void Client::SetPref(std::string property, int value)
{
	std::stringstream hexInt;
	hexInt << std::hex << value;
	json::UnknownElement intValue = json::String(hexInt.str());
	SetPref(property, intValue);
}

void Client::SetPref(std::string property, unsigned int value)
{
	std::stringstream hexInt;
	hexInt << std::hex << value;
	json::UnknownElement intValue = json::String(hexInt.str());
	SetPref(property, intValue);
}

void Client::SetPref(std::string property, std::vector<std::string> value)
{
	json::Array newArray;
	for(std::vector<std::string>::iterator iter = value.begin(); iter != value.end(); ++iter)
	{
		newArray.Insert(json::String(*iter));
	}
	json::UnknownElement newArrayValue = newArray;
	SetPref(property, newArrayValue);
}

void Client::SetPref(std::string property, std::vector<double> value)
{
	json::Array newArray;
	for(std::vector<double>::iterator iter = value.begin(); iter != value.end(); ++iter)
	{
		newArray.Insert(json::Number(*iter));
	}
	json::UnknownElement newArrayValue = newArray;
	SetPref(property, newArrayValue);
}

void Client::SetPref(std::string property, std::vector<bool> value)
{
	json::Array newArray;
	for(std::vector<bool>::iterator iter = value.begin(); iter != value.end(); ++iter)
	{
		newArray.Insert(json::Boolean(*iter));
	}
	json::UnknownElement newArrayValue = newArray;
	SetPref(property, newArrayValue);
}

void Client::SetPref(std::string property, std::vector<int> value)
{
	json::Array newArray;
	for(std::vector<int>::iterator iter = value.begin(); iter != value.end(); ++iter)
	{
		std::stringstream hexInt;
		hexInt << std::hex << *iter;

		newArray.Insert(json::String(hexInt.str()));
	}
	json::UnknownElement newArrayValue = newArray;
	SetPref(property, newArrayValue);
}

void Client::SetPref(std::string property, std::vector<unsigned int> value)
{
	json::Array newArray;
	for(std::vector<unsigned int>::iterator iter = value.begin(); iter != value.end(); ++iter)
	{
		std::stringstream hexInt;
		hexInt << std::hex << *iter;

		newArray.Insert(json::String(hexInt.str()));
	}
	json::UnknownElement newArrayValue = newArray;
	SetPref(property, newArrayValue);
}

void Client::SetPref(std::string property, bool value)
{
	json::UnknownElement boolValue = json::Boolean(value);
	SetPref(property, boolValue);
}

json::UnknownElement Client::GetPref(std::string property)
{
	std::vector<std::string> pTokens = Client::explodePropertyString(property);
	const json::UnknownElement & configDocumentCopy = configDocument;
	json::UnknownElement currentRef = configDocumentCopy;
	for(std::vector<std::string>::iterator iter = pTokens.begin(); iter != pTokens.end(); ++iter)
	{
		currentRef = ((const json::UnknownElement &)currentRef)[*iter];
	}
	return currentRef;
}

void Client::setPrefR(std::deque<std::string> tokens, json::UnknownElement & element, json::UnknownElement & value)
{
	if(tokens.size())
	{
		std::string token = tokens.front();
		tokens.pop_front();
		setPrefR(tokens, element[token], value);
	}
	else
		element = value;
}

void Client::SetPref(std::string property, json::UnknownElement & value)
{
	std::vector<std::string> pTokens = Client::explodePropertyString(property);
	std::deque<std::string> dTokens(pTokens.begin(), pTokens.end());
	std::string token = dTokens.front();
	dTokens.pop_front();
	setPrefR(dTokens, configDocument[token], value);
}
