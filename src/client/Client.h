#pragma once
#include "common/Bson.h"
#include "common/String.h"
#include "common/ExplicitSingleton.h"
#include "StartupInfo.h"
#include "User.h"
#include <vector>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>

class SaveInfo;
class SaveFile;
class GameSave;
class VideoBuffer;

class Prefs;
class RequestListener;
class ClientListener;
namespace http
{
	class StartupRequest;
}
class Client: public ExplicitSingleton<Client> {
public:
	enum class StartupRequestStatus
	{
		notYetDone,
		inProgress,
		succeeded,
		failed,
	};

private:
	bool autoStartupRequest = true;
	String messageOfTheDay;
	std::vector<ServerNotification> serverNotifications;

	std::unique_ptr<http::StartupRequest> versionCheckRequest;
	std::unique_ptr<http::StartupRequest> alternateVersionCheckRequest;
	bool usingAltUpdateServer;
	bool updateAvailable;
	std::optional<UpdateInfo> updateInfo;

	bool firstRun;
	bool redirectStd = false;

	std::vector<ByteString> stampIDs;
	uint64_t lastStampTime = 0;
	int lastStampName = 0;

	//Auth session
	std::optional<User> authUser;

	void notifyUpdateAvailable();
	void notifyAuthUserChanged();
	void notifyMessageOfTheDay();
	void notifyNewNotification(ServerNotification notification);

	// Save stealing info
	Bson authors;

	std::unique_ptr<Prefs> stamps;
	void MigrateStampsDef();
	void WriteStamps();

	void LoadAuthUser();
	void SaveAuthUser();

	StartupRequestStatus startupRequestStatus = StartupRequestStatus::notYetDone;
	std::optional<ByteString> startupRequestError;

public:

	std::vector<ClientListener*> listeners;

	// Save stealing info
	void MergeStampAuthorInfo(const Bson &linksToAdd);
	void MergeAuthorInfo(const Bson &linksToAdd);
	void OverwriteAuthorInfo(Bson overwrite) { authors = std::move(overwrite); }
	const Bson &GetAuthorInfo() const { return authors; }
	void SaveAuthorInfo(Bson &saveInto) const;
	void ClearAuthorInfo() { authors = Bson{}; }
	bool IsAuthorsEmpty() const { return authors.GetSize() == 0; }

	std::optional<UpdateInfo> GetUpdateInfo();

	Client();
	~Client();

	ByteString FileOpenDialogue();
	//std::string FileSaveDialogue();

	void AddServerNotification(ServerNotification notification);
	std::vector<ServerNotification> GetServerNotifications();

	void SetMessageOfTheDay(String message);
	String GetMessageOfTheDay();

	void Initialize();
	bool IsFirstRun();

	void AddListener(ClientListener * listener);
	void RemoveListener(ClientListener * listener);

	std::unique_ptr<SaveFile> GetStamp(ByteString stampID);
	void DeleteStamp(ByteString stampID);
	void RenameStamp(ByteString stampID, ByteString newName);
	ByteString AddStamp(std::unique_ptr<GameSave> saveData);
	void RescanStamps();
	const std::vector<ByteString> &GetStamps() const;
	void MoveStampToFront(ByteString stampID);

	std::unique_ptr<SaveFile> LoadSaveFile(ByteString filename);

	void SetAuthUser(std::optional<User> user);
	const std::optional<User> &GetAuthUser() const;
	void Tick();
	
	String DoMigration(ByteString fromDir, ByteString toDir);

	bool GetRedirectStd()
	{
		return redirectStd;
	}

	void SetRedirectStd(bool newRedirectStd)
	{
		redirectStd = newRedirectStd;
	}

	bool GetAutoStartupRequest()
	{
		return autoStartupRequest;
	}

	void SetAutoStartupRequest(bool newAutoStartupRequest)
	{
		autoStartupRequest = newAutoStartupRequest;
	}

	void BeginStartupRequest();
	StartupRequestStatus GetStartupRequestStatus() const
	{
		return startupRequestStatus;
	}
	std::optional<ByteString> GetStartupRequestError() const
	{
		return startupRequestError;
	}
};
