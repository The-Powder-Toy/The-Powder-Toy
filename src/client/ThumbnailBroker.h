#pragma once
#include <queue>
#include <list>
#include <utility>
#include <deque>
#include <pthread.h>
#undef GetUserName //God dammit microsoft!

#include "Singleton.h"

class GameSave;
class Thumbnail;
class ThumbnailListener;
class ThumbnailBroker: public Singleton<ThumbnailBroker>
{
private: 
	class ThumbnailID;
	class ThumbnailRequest;
	class ThumbnailSpec;
	class ThumbRenderRequest;

	//Thumbnail retreival
	/*int thumbnailCacheNextID;
	Thumbnail * thumbnailCache[THUMB_CACHE_SIZE];
	void * activeThumbRequests[IMGCONNS];
	int activeThumbRequestTimes[IMGCONNS];
	int activeThumbRequestCompleteTimes[IMGCONNS];
	std::string activeThumbRequestIDs[IMGCONNS];*/

	pthread_mutex_t thumbnailQueueMutex;
	pthread_mutex_t listenersMutex;
	pthread_t thumbnailQueueThread;
	bool thumbnailQueueRunning;
	std::deque<ThumbnailRequest> thumbnailRequests;
	std::deque<ThumbRenderRequest> renderRequests; 

	std::deque<std::pair<ThumbnailListener*, Thumbnail*> > thumbnailComplete;
	std::list<ThumbnailRequest> currentRequests;
	std::deque<std::pair<ThumbnailID, Thumbnail*> > thumbnailCache;

	std::vector<ThumbnailListener*> validListeners;

	static void * thumbnailQueueProcessHelper(void * ref);
	void thumbnailQueueProcessTH();

public:
	ThumbnailBroker();
	virtual ~ThumbnailBroker();

	void FlushThumbQueue();
	void RenderThumbnail(GameSave * gameSave, bool decorations, int width, int height, ThumbnailListener * tListener);
	void RenderThumbnail(GameSave * gameSave, int width, int height, ThumbnailListener * tListener);
	void RetrieveThumbnail(int saveID, int saveDate, int width, int height, ThumbnailListener * tListener);
	void RetrieveThumbnail(int saveID, int width, int height, ThumbnailListener * tListener);
	
	bool CheckThumbnailListener(ThumbnailListener * tListener);
	void AttachThumbnailListener(ThumbnailListener * tListener);
	void DetachThumbnailListener(ThumbnailListener * tListener);
};