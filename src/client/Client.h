#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include <vector>
#include <list>

#include "Config.h"
#include "Singleton.h"

#include "User.h"
#include "UserInfo.h"

#include "cajun/elements.h"

#include "requestbroker/RequestBroker.h"

class Thumbnail;
class SaveInfo;
class SaveFile;
class SaveComment;
class GameSave;
class VideoBuffer;

enum LoginStatus {
	LoginOkay, LoginError
};

enum RequestStatus {
	RequestOkay, RequestFailure
};

class UpdateInfo
{
public:
	enum BuildType { Stable, Beta, Snapshot };
	std::string File;
	int Major;
	int Minor;
	int Build;
	int Time;
	BuildType Type;
	UpdateInfo() : Major(0), Minor(0), Build(0), Time(0), File(""), Type(Stable) {}
	UpdateInfo(int major, int minor, int build, std::string file, BuildType type) : Major(major), Minor(minor), Build(build), Time(0), File(file), Type(type) {}
	UpdateInfo(int time, std::string file, BuildType type) : Major(0), Minor(0), Build(0), Time(time), File(file), Type(type) {}
};

class RequestListener;
class ClientListener;
class Client: public Singleton<Client> {
private:
	std::string messageOfTheDay;
	std::vector<std::pair<std::string, std::string> > serverNotifications; 

	void * versionCheckRequest;
	bool updateAvailable;
	UpdateInfo updateInfo;


	std::string lastError;

	std::list<std::string> stampIDs;
	int lastStampTime;
	int lastStampName;

	//Auth session
	User authUser;

	//Thumbnail retreival
	int thumbnailCacheNextID;
	Thumbnail * thumbnailCache[THUMB_CACHE_SIZE];
	void * activeThumbRequests[IMGCONNS];
	int activeThumbRequestTimes[IMGCONNS];
	int activeThumbRequestCompleteTimes[IMGCONNS];
	std::string activeThumbRequestIDs[IMGCONNS];
	void updateStamps();
	static std::vector<std::string> explodePropertyString(std::string property);
	void notifyUpdateAvailable();
	void notifyAuthUserChanged();
	void notifyMessageOfTheDay();
	void notifyNewNotification(std::pair<std::string, std::string> notification);

	//Config file handle
	json::Object configDocument;
public:

	std::vector<ClientListener*> listeners;

	UpdateInfo GetUpdateInfo();

	Client();
	~Client();

	std::vector<std::string> DirectorySearch(std::string directory, std::string search, std::vector<std::string> extensions);
	std::vector<std::string> DirectorySearch(std::string directory, std::string search, std::string extension);

	std::string FileOpenDialogue();
	//std::string FileSaveDialogue();

	bool DoInstallation();

	std::vector<unsigned char> ReadFile(std::string filename);

	void AddServerNotification(std::pair<std::string, std::string> notification);
	std::vector<std::pair<std::string, std::string> > GetServerNotifications();

	void SetMessageOfTheDay(std::string message);
	std::string GetMessageOfTheDay();

	void Initialise(std::string proxyString);
	void SetProxy(std::string proxy);

	int MakeDirectory(const char * dirname);
	void WriteFile(std::vector<unsigned char> fileData, std::string filename);
	void WriteFile(std::vector<char> fileData, std::string filename);
	bool FileExists(std::string filename);

	void AddListener(ClientListener * listener);
	void RemoveListener(ClientListener * listener);

	RequestStatus ExecVote(int saveID, int direction);
	RequestStatus UploadSave(SaveInfo & save);

	SaveFile * GetStamp(std::string stampID);
	void DeleteStamp(std::string stampID);
	std::string AddStamp(GameSave * saveData);
	std::vector<std::string> GetStamps(int start, int count);
	void RescanStamps();
	int GetStampsCount();
	SaveFile * GetFirstStamp();
	void MoveStampToFront(std::string stampID);

	RequestStatus AddComment(int saveID, std::string comment);

	//Retrieves a "UserInfo" object
	RequestBroker::Request * GetUserInfoAsync(std::string username);
	RequestBroker::Request * SaveUserInfoAsync(UserInfo info);

	unsigned char * GetSaveData(int saveID, int saveDate, int & dataLength);
	std::vector<unsigned char> GetSaveData(int saveID, int saveDate);
	LoginStatus Login(std::string username, std::string password, User & user);
	void ClearThumbnailRequests();
	std::vector<SaveInfo*> * SearchSaves(int start, int count, std::string query, std::string sort, std::string category, int & resultCount);
	std::vector<std::pair<std::string, int> > * GetTags(int start, int count, std::string query, int & resultCount);
	std::vector<SaveComment*> * GetComments(int saveID, int start, int count);
	Thumbnail * GetPreview(int saveID, int saveDate);
	Thumbnail * GetThumbnail(int saveID, int saveDate);
	SaveInfo * GetSave(int saveID, int saveDate);
	RequestStatus DeleteSave(int saveID);
	RequestStatus ReportSave(int saveID, std::string message);
	RequestStatus UnpublishSave(int saveID);
	RequestStatus FavouriteSave(int saveID, bool favourite);
	void SetAuthUser(User user);
	User GetAuthUser();
	std::vector<std::string> * RemoveTag(int saveID, std::string tag); //TODO RequestStatus
	std::vector<std::string> * AddTag(int saveID, std::string tag);
	std::string GetLastError() {
		return lastError;
	}
	void Tick();
	void Shutdown();

	//Force flushing preferences to file on disk.
	void WritePrefs();

	std::string GetPrefString(std::string property, std::string defaultValue);
	double GetPrefNumber(std::string property, double defaultValue);
	int GetPrefInteger(std::string property, int defaultValue);
	unsigned int GetPrefUInteger(std::string property, unsigned int defaultValue);
	std::vector<std::string> GetPrefStringArray(std::string property);
	std::vector<double> GetPrefNumberArray(std::string property);
	std::vector<int> GetPrefIntegerArray(std::string property);
	std::vector<unsigned int> GetPrefUIntegerArray(std::string property);
	std::vector<bool> GetPrefBoolArray(std::string property);
	bool GetPrefBool(std::string property, bool defaultValue);

	void SetPref(std::string property, std::string value);
	void SetPref(std::string property, double value);
	void SetPref(std::string property, int value);
	void SetPref(std::string property, unsigned int value);
	void SetPref(std::string property, std::vector<std::string> value);
	void SetPref(std::string property, std::vector<double> value);
	void SetPref(std::string property, std::vector<int> value);
	void SetPref(std::string property, std::vector<unsigned int> value);
	void SetPref(std::string property, std::vector<bool> value);
	void SetPref(std::string property, bool value);

	json::UnknownElement GetPref(std::string property);
	void setPrefR(std::deque<std::string> tokens, json::UnknownElement & element, json::UnknownElement & value);
	void SetPref(std::string property, json::UnknownElement & value);
};

#endif // CLIENT_H
