#pragma once
#include "common/String.h"
#include "common/ExplicitSingleton.h"
#include "User.h"
#include <vector>
#include <cstdint>
#include <list>
#include <memory>
#include <json/json.h>

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

class Prefs;
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

	std::vector<ByteString> stampIDs;
	uint64_t lastStampTime = 0;
	int lastStampName = 0;

	//Auth session
	User authUser;

	void notifyUpdateAvailable();
	void notifyAuthUserChanged();
	void notifyMessageOfTheDay();
	void notifyNewNotification(std::pair<String, ByteString> notification);

	// Save stealing info
	Json::Value authors;

	std::unique_ptr<Prefs> stamps;
	void MigrateStampsDef();
	void WriteStamps();

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

	std::unique_ptr<SaveFile> GetStamp(ByteString stampID);
	void DeleteStamp(ByteString stampID);
	ByteString AddStamp(std::unique_ptr<GameSave> saveData);
	void RescanStamps();
	const std::vector<ByteString> &GetStamps() const;
	void MoveStampToFront(ByteString stampID);

	RequestStatus AddComment(int saveID, String comment);

	std::vector<char> GetSaveData(int saveID, int saveDate);

	LoginStatus Login(ByteString username, ByteString password, User & user);

	std::unique_ptr<SaveInfo> GetSave(int saveID, int saveDate);
	std::unique_ptr<SaveFile> LoadSaveFile(ByteString filename);

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
