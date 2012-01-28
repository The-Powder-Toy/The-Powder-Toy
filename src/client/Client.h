#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include <vector>

#include "Config.h"
#include "HTTP.h"
#include "search/Thumbnail.h"
#include "search/Save.h"
#include "Singleton.h"
#include "User.h"

enum LoginStatus {
	LoginOkay, LoginError
};

enum RequestStatus {
	RequestOkay, RequestFailure
};

class Client: public Singleton<Client> {
private:
	std::string lastError;

	//Auth session
	User authUser;

	//Thumbnail retreival
	int thumbnailCacheNextID;
	Thumbnail * thumbnailCache[THUMB_CACHE_SIZE];
	void * activeThumbRequests[IMGCONNS];
	int activeThumbRequestTimes[IMGCONNS];
	int activeThumbRequestCompleteTimes[IMGCONNS];
	std::string activeThumbRequestIDs[IMGCONNS];
public:
	Client();
	~Client();

	RequestStatus ExecVote(int saveID, int direction);

	unsigned char * GetSaveData(int saveID, int saveDate, int & dataLength);
	LoginStatus Login(string username, string password, User & user);
	void ClearThumbnailRequests();
	std::vector<Save*> * SearchSaves(int start, int count, string query, string sort, int & resultCount);
	Thumbnail * GetPreview(int saveID, int saveDate);
	Thumbnail * GetThumbnail(int saveID, int saveDate);
	Save * GetSave(int saveID, int saveDate);
	void SetAuthUser(User user);
	User GetAuthUser();
	std::string GetLastError() {
		return lastError;
	}
};

#endif // CLIENT_H
