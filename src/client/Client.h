#ifndef CLIENT_H
#define CLIENT_H

#include <queue>
#include "Config.h"
#include "HTTP.h"
#include "search/Thumbnail.h"
#include "Singleton.h"

class Client: public Singleton<Client>
{
private:
	int thumbnailCacheNextID;
	Thumbnail * thumbnailCache[THUMB_CACHE_SIZE];
	void * activeThumbRequests[IMGCONNS];
	int activeThumbRequestTimes[IMGCONNS];
	int activeThumbRequestCompleteTimes[IMGCONNS];
	std::string activeThumbRequestIDs[IMGCONNS];
public:
	Client();
	~Client();
	void ClearThumbnailRequests();
	Thumbnail * GetThumbnail(int saveID, int saveDate);
};

#endif // CLIENT_H
