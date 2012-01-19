#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include "HTTP.h"
#include "search/Thumbnail.h"
#include "Singleton.h"

class Client: public Singleton<Client>
{
private:
	int thumbnailCacheNextID;
	Thumbnail * thumbnailCache[120];
	void * activeThumbRequests[5];
	int activeThumbRequestTimes[5];
	std::string activeThumbRequestIDs[5];
public:
	Client();
	~Client();
	void ClearThumbnailRequests();
	Thumbnail * GetThumbnail(int saveID, int saveDate);
};

#endif // CLIENT_H
