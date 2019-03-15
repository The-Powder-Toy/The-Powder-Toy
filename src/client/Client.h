#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include <vector>
#include <list>

#include "common/String.h"
#include "Config.h"
#include "common/Singleton.h"

#include "User.h"
#include "UserInfo.h"

#include "json/json.h"

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
	ByteString File;
	String Changelog;
	int Major;
	int Minor;
	int Build;
	int Time;
	BuildType Type;
	UpdateInfo() : File(""), Changelog(""), Major(0), Minor(0), Build(0), Time(0), Type(Stable) {}
	UpdateInfo(int major, int minor, int build, ByteString file, String changelog, BuildType type) : File(file), Changelog(changelog), Major(major), Minor(minor), Build(build), Time(0), Type(type) {}
	UpdateInfo(int time, ByteString file, String changelog, BuildType type) : File(file), Changelog(changelog), Major(0), Minor(0), Build(0), Time(time), Type(type) {}
};

class RequestListener;
class ClientListener;
namespace http
{
	class Request;
}
class Client: public Singleton<Client> {
private:
	String messageOfTheDay;
	std::vector<std::pair<String, ByteString> > serverNotifications;

	http::Request *versionCheckRequest;
	http::Request *alternateVersionCheckRequest;
	bool usingAltUpdateServer;
	bool updateAvailable;
	UpdateInfo updateInfo;

	String lastError;
	bool firstRun;

	std::list<ByteString> stampIDs;
	unsigned lastStampTime;
	int lastStampName;

	//Auth session
	User authUser;

	void notifyUpdateAvailable();
	void notifyAuthUserChanged();
	void notifyMessageOfTheDay();
	void notifyNewNotification(std::pair<String, ByteString> notification);

	// internal preferences handling
	Json::Value preferences;
	Json::Value GetPref(Json::Value root, ByteString prop, Json::Value defaultValue = Json::nullValue);
	Json::Value SetPrefHelper(Json::Value root, ByteString prop, Json::Value value);

	// Save stealing info
	Json::Value authors;

public:

	std::vector<ClientListener*> listeners;

	// Save stealing info
	void MergeStampAuthorInfo(Json::Value linksToAdd);
	void MergeAuthorInfo(Json::Value linksToAdd);
	void OverwriteAuthorInfo(Json::Value overwrite) { authors = overwrite; }
	Json::Value GetAuthorInfo() { return authors; }
	void SaveAuthorInfo(Json::Value *saveInto);
	void ClearAuthorInfo() { authors.clear(); }
	bool IsAuthorsEmpty() { return authors.size() == 0; }

	UpdateInfo GetUpdateInfo();

	Client();
	~Client();

	std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions);
	std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, ByteString extension);

	ByteString FileOpenDialogue();
	//std::string FileSaveDialogue();

	bool DoInstallation();

	std::vector<unsigned char> ReadFile(ByteString filename);

	void AddServerNotification(std::pair<String, ByteString> notification);
	std::vector<std::pair<String, ByteString> > GetServerNotifications();

	void SetMessageOfTheDay(String message);
	String GetMessageOfTheDay();

	void Initialise(ByteString proxyString);
	bool IsFirstRun();

	int MakeDirectory(const char * dirname);
	bool WriteFile(std::vector<unsigned char> fileData, ByteString filename);
	bool WriteFile(std::vector<char> fileData, ByteString filename);
	bool FileExists(ByteString filename);

	void AddListener(ClientListener * listener);
	void RemoveListener(ClientListener * listener);

	RequestStatus ExecVote(int saveID, int direction);
	RequestStatus UploadSave(SaveInfo & save);

	SaveFile * GetStamp(ByteString stampID);
	void DeleteStamp(ByteString stampID);
	ByteString AddStamp(GameSave * saveData);
	std::vector<ByteString> GetStamps(int start, int count);
	void RescanStamps();
	int GetStampsCount();
	SaveFile * GetFirstStamp();
	void MoveStampToFront(ByteString stampID);
	void updateStamps();

	RequestStatus AddComment(int saveID, String comment);

	std::vector<unsigned char> GetSaveData(int saveID, int saveDate);

	LoginStatus Login(ByteString username, ByteString password, User & user);
	std::vector<SaveInfo*> * SearchSaves(int start, int count, String query, ByteString sort, ByteString category, int & resultCount);
	std::vector<std::pair<ByteString, int> > * GetTags(int start, int count, String query, int & resultCount);

	SaveInfo * GetSave(int saveID, int saveDate);

	RequestStatus DeleteSave(int saveID);
	RequestStatus ReportSave(int saveID, String message);
	RequestStatus UnpublishSave(int saveID);
	RequestStatus PublishSave(int saveID);
	RequestStatus FavouriteSave(int saveID, bool favourite);
	void SetAuthUser(User user);
	User GetAuthUser();
	std::list<ByteString> * RemoveTag(int saveID, ByteString tag); //TODO RequestStatus
	std::list<ByteString> * AddTag(int saveID, ByteString tag);
	String GetLastError() {
		return lastError;
	}
	RequestStatus ParseServerReturn(ByteString &result, int status, bool json);
	void Tick();
	bool CheckUpdate(http::Request *updateRequest, bool checkSession);
	void Shutdown();

	// preferences functions
	void WritePrefs();

	ByteString GetPrefByteString(ByteString prop, ByteString defaultValue);
	String GetPrefString(ByteString prop, String defaultValue);
	double GetPrefNumber(ByteString prop, double defaultValue);
	int GetPrefInteger(ByteString prop, int defaultValue);
	unsigned int GetPrefUInteger(ByteString prop, unsigned int defaultValue);
	bool GetPrefBool(ByteString prop, bool defaultValue);
	std::vector<ByteString> GetPrefByteStringArray(ByteString prop);
	std::vector<String> GetPrefStringArray(ByteString prop);
	std::vector<double> GetPrefNumberArray(ByteString prop);
	std::vector<int> GetPrefIntegerArray(ByteString prop);
	std::vector<unsigned int> GetPrefUIntegerArray(ByteString prop);
	std::vector<bool> GetPrefBoolArray(ByteString prop);

	void SetPref(ByteString prop, Json::Value value);
	void SetPref(ByteString property, std::vector<Json::Value> value);
	void SetPrefUnicode(ByteString prop, String value);
};

#endif // CLIENT_H
