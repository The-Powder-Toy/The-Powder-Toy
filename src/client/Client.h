#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include <vector>
#include <list>
#include <fstream>

#include "Config.h"
#include "HTTP.h"
#include "preview/Comment.h"
#include "search/Thumbnail.h"
#include "client/SaveInfo.h"
#include "client/SaveFile.h"
#include "Singleton.h"
#include "User.h"

#include "cajun/reader.h"
#include "cajun/writer.h"
#include "cajun/elements.h"

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

class ThumbnailListener;
class ClientListener;
class Client: public Singleton<Client> {
private:
	void * versionCheckRequest;
	bool updateAvailable;
	UpdateInfo updateInfo;


	std::string lastError;

	list<string> stampIDs;
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
	static vector<std::string> explodePropertyString(std::string property);
	void notifyUpdateAvailable();
	void notifyAuthUserChanged();

	//Config file handle
	json::Object configDocument;
public:

	vector<ClientListener*> listeners;

	UpdateInfo GetUpdateInfo();

	Client();
	~Client();

	std::vector<std::string> DirectorySearch(std::string directory, std::string search, std::vector<std::string> extensions);
	std::vector<std::string> DirectorySearch(std::string directory, std::string search, std::string extension);

	std::vector<unsigned char> ReadFile(std::string filename);

	void WriteFile(std::vector<unsigned char> fileData, std::string filename);
	void WriteFile(std::vector<char> fileData, std::string filename);
	bool FileExists(std::string filename);

	void AddListener(ClientListener * listener);
	void RemoveListener(ClientListener * listener);

	RequestStatus ExecVote(int saveID, int direction);
	RequestStatus UploadSave(SaveInfo & save);

	SaveFile * GetStamp(string stampID);
	void DeleteStamp(string stampID);
	string AddStamp(GameSave * saveData);
	vector<string> GetStamps(int start, int count);
	int GetStampsCount();
	SaveFile * GetFirstStamp();

	RequestStatus AddComment(int saveID, std::string comment);

	unsigned char * GetSaveData(int saveID, int saveDate, int & dataLength);
	LoginStatus Login(string username, string password, User & user);
	void ClearThumbnailRequests();
	std::vector<SaveInfo*> * SearchSaves(int start, int count, string query, string sort, string category, int & resultCount);
	std::vector<std::pair<std::string, int> > * GetTags(int start, int count, string query, int & resultCount);
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
	std::vector<string> * RemoveTag(int saveID, string tag); //TODO RequestStatus
	std::vector<string> * AddTag(int saveID, string tag);
	std::string GetLastError() {
		return lastError;
	}
	void Tick();
	void Shutdown();

	std::string GetPrefString(std::string property, std::string defaultValue);
	double GetPrefNumber(std::string property, double defaultValue);
	int GetPrefInteger(std::string property, int defaultValue);
	unsigned int GetPrefUInteger(std::string property, unsigned int defaultValue);
	vector<string> GetPrefStringArray(std::string property);
	vector<double> GetPrefNumberArray(std::string property);
	vector<int> GetPrefIntegerArray(std::string property);
	vector<unsigned int> GetPrefUIntegerArray(std::string property);
	vector<bool> GetPrefBoolArray(std::string property);
	bool GetPrefBool(std::string property, bool defaultValue);

	void SetPref(std::string property, std::string value);
	void SetPref(std::string property, double value);
	void SetPref(std::string property, int value);
	void SetPref(std::string property, unsigned int value);
	void SetPref(std::string property, vector<string> value);
	void SetPref(std::string property, vector<double> value);
	void SetPref(std::string property, vector<int> value);
	void SetPref(std::string property, vector<unsigned int> value);
	void SetPref(std::string property, vector<bool> value);
	void SetPref(std::string property, bool value);

	json::UnknownElement GetPref(std::string property);
	void setPrefR(std::deque<string> tokens, json::UnknownElement & element, json::UnknownElement & value);
	void SetPref(std::string property, json::UnknownElement & value);
};

#endif // CLIENT_H
