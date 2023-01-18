#pragma once
#include "Config.h"

#include <vector>
#include <list>
#include <memory>

#include "common/String.h"
#include "common/ExplicitSingleton.h"
#include <json/json.h>

#include "User.h"

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
class Client: public ExplicitSingleton<Client> {
private:
	String messageOfTheDay;
	std::vector<std::pair<String, ByteString> > serverNotifications;

	std::unique_ptr<http::Request> versionCheckRequest;
	std::unique_ptr<http::Request> alternateVersionCheckRequest;
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

	ByteString FileOpenDialogue();
	//std::string FileSaveDialogue();

	bool DoInstallation();

	void AddServerNotification(std::pair<String, ByteString> notification);
	std::vector<std::pair<String, ByteString> > GetServerNotifications();

	void SetMessageOfTheDay(String message);
	String GetMessageOfTheDay();

	void Initialize();
	bool IsFirstRun();

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

	std::vector<char> GetSaveData(int saveID, int saveDate);

	LoginStatus Login(ByteString username, ByteString password, User & user);
	std::vector<SaveInfo*> * SearchSaves(int start, int count, String query, ByteString sort, ByteString category, int & resultCount);
	std::vector<std::pair<ByteString, int> > * GetTags(int start, int count, String query, int & resultCount);

	SaveInfo * GetSave(int saveID, int saveDate);
	SaveFile * LoadSaveFile(ByteString filename);

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
	void CheckUpdate(std::unique_ptr<http::Request> &updateRequest, bool checkSession);
	
	String DoMigration(ByteString fromDir, ByteString toDir);
};

bool AddCustomGol(String ruleString, String nameString, unsigned int highColor, unsigned int lowColor);
