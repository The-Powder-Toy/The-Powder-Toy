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

class ClientListener;
class Client: public Singleton<Client> {
private:
	void * versionCheckRequest;
	bool updateAvailable;


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

	//Config file handle
	json::Object configDocument;
public:
	vector<ClientListener*> listeners;

	Client();
	~Client();

	void AddListener(ClientListener * listener);

	RequestStatus ExecVote(int saveID, int direction);
	RequestStatus UploadSave(SaveInfo * save);

	SaveFile * GetStamp(string stampID);
	void DeleteStamp(string stampID);
	string AddStamp(GameSave * saveData);
	vector<string> GetStamps(int start, int count);
	int GetStampsCount();
	SaveFile * GetFirstStamp();

	unsigned char * GetSaveData(int saveID, int saveDate, int & dataLength);
	LoginStatus Login(string username, string password, User & user);
	void ClearThumbnailRequests();
	std::vector<SaveInfo*> * SearchSaves(int start, int count, string query, string sort, string category, int & resultCount);
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
	vector<string> GetPrefStringArray(std::string property);
	vector<double> GetPrefNumberArray(std::string property);
	vector<bool> GetPrefBoolArray(std::string property);
	bool GetPrefBool(std::string property, bool defaultValue);

	void SetPref(std::string property, std::string value);
	void SetPref(std::string property, double value);
	void SetPref(std::string property, vector<string> value);
	void SetPref(std::string property, vector<double> value);
	void SetPref(std::string property, vector<bool> value);
	void SetPref(std::string property, bool value);

	json::UnknownElement GetPref(std::string property);
	void setPrefR(std::deque<string> tokens, json::UnknownElement & element, json::UnknownElement & value);
	void SetPref(std::string property, json::UnknownElement & value);
};

#endif // CLIENT_H
