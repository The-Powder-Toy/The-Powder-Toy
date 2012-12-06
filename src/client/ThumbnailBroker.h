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
typedef std::pair<int, ThumbnailListener*> ListenerHandle;
class ThumbnailBroker: public Singleton<ThumbnailBroker>
{
private: 
	class ThumbnailSpec
	{
	public:
		int Width, Height;
		ListenerHandle CompletedListener;
		ThumbnailSpec(int width, int height, ListenerHandle completedListener) :
			Width(width), Height(height), CompletedListener(completedListener) {}
	};

	class ThumbnailID
	{
	public:
		int SaveID, SaveDate;
		bool operator ==(const ThumbnailID & second)
		{
			return SaveID == second.SaveID && SaveDate == second.SaveDate;
		}
		ThumbnailID(int saveID, int saveDate) : SaveID(saveID), SaveDate(saveDate) {}
		ThumbnailID() : SaveID(0), SaveDate(0) {}
	};

	class ThumbnailRequest
	{
	public:
		bool Complete;
		void * HTTPContext;
		int RequestTime;

		ThumbnailID ID;
		std::vector<ThumbnailSpec> SubRequests;
		
		ThumbnailRequest(int saveID, int saveDate, int width, int height, ListenerHandle completedListener) :
			ID(saveID, saveDate), Complete(false), HTTPContext(NULL), RequestTime(0)
			{
				SubRequests.push_back(ThumbnailSpec(width, height, completedListener));
			}
		ThumbnailRequest() : Complete(false), HTTPContext(NULL), RequestTime(0) {}
	};

	class ThumbRenderRequest
	{
	public:
		int Width, Height;
		bool Decorations;
		bool Fire;
		GameSave * Save;
		ListenerHandle CompletedListener;
		ThumbRenderRequest(GameSave * save, bool decorations, bool fire, int width, int height, ListenerHandle completedListener) :
			Save(save), Width(width), Height(height), CompletedListener(completedListener), Decorations(decorations), Fire(fire) {}
		ThumbRenderRequest() :	Save(0), Decorations(true), Fire(true), Width(0), Height(0), CompletedListener(ListenerHandle(0, (ThumbnailListener*)NULL)) {}
	};

	//Thumbnail retreival
	/*int thumbnailCacheNextID;
	Thumbnail * thumbnailCache[THUMB_CACHE_SIZE];
	void * activeThumbRequests[IMGCONNS];
	int activeThumbRequestTimes[IMGCONNS];
	int activeThumbRequestCompleteTimes[IMGCONNS];
	std::string activeThumbRequestIDs[IMGCONNS];*/

	pthread_mutex_t thumbnailQueueMutex;
	pthread_mutex_t listenersMutex;
	pthread_mutex_t runningMutex;
	pthread_t thumbnailQueueThread;
	bool thumbnailQueueRunning;
	std::deque<ThumbnailRequest> thumbnailRequests;
	std::deque<ThumbRenderRequest> renderRequests; 

	std::deque<std::pair<ListenerHandle, Thumbnail*> > thumbnailComplete;
	std::list<ThumbnailRequest> currentRequests;
	std::deque<std::pair<ThumbnailID, Thumbnail*> > thumbnailCache;


	std::vector<ListenerHandle> validListeners;

	static void * thumbnailQueueProcessHelper(void * ref);
	void thumbnailQueueProcessTH();
	void assureRunning();

public:
	ThumbnailBroker();
	virtual ~ThumbnailBroker();
	void Shutdown();

	void FlushThumbQueue();
	void RenderThumbnail(GameSave * gameSave, bool decorations, bool fire, int width, int height, ThumbnailListener * tListener);
	void RenderThumbnail(GameSave * gameSave, int width, int height, ThumbnailListener * tListener);
	void RetrieveThumbnail(int saveID, int saveDate, int width, int height, ThumbnailListener * tListener);
	void RetrieveThumbnail(int saveID, int width, int height, ThumbnailListener * tListener);
	
	bool CheckThumbnailListener(ListenerHandle handle);
	void AttachThumbnailListener(ThumbnailListener * tListener);
	void DetachThumbnailListener(ThumbnailListener * tListener);
};