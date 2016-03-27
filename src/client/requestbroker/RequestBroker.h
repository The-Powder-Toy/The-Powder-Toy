#pragma once
#include <queue>
#include <list>
#include <utility>
#include <deque>
#include <string>
#include "common/tpt-thread.h"

#include "Config.h"
#include "common/Singleton.h"

class GameSave;
class VideoBuffer;
class RequestListener;
typedef std::pair<int, RequestListener*> ListenerHandle;
class RequestBroker: public Singleton<RequestBroker>
{
	friend class ImageRequest;
	friend class APIRequest;
	friend class WebRequest;
	friend class ThumbRenderRequest;
public:
	class Request;
private: 

	pthread_mutex_t listenersMutex;
	pthread_mutex_t runningMutex;
	pthread_mutex_t requestQueueMutex;
	pthread_mutex_t completeQueueMutex;

	pthread_t thumbnailQueueThread;
	bool thumbnailQueueRunning;

	std::vector<ListenerHandle> validListeners;

	std::deque<std::pair<std::string, VideoBuffer*> > imageCache;

	std::queue<Request*> completeQueue;
	std::vector<Request*> requestQueue;
	std::vector<Request*> activeRequests;

	TH_ENTRY_POINT static void * thumbnailQueueProcessHelper(void * ref);
	void thumbnailQueueProcessTH();
	void assureRunning();

	void requestComplete(Request * completedRequest);

public:
	RequestBroker();
	virtual ~RequestBroker();
	void Shutdown();

	void FlushThumbQueue();
	void RetrieveImage(std::string imageUrl, int width, int height, RequestListener * tListener);
	void RenderThumbnail(GameSave * gameSave, bool decorations, bool fire, int width, int height, RequestListener * tListener);
	void RenderThumbnail(GameSave * gameSave, int width, int height, RequestListener * tListener);
	void RetrieveThumbnail(int saveID, int saveDate, int width, int height, RequestListener * tListener);
	void RetrieveThumbnail(int saveID, int width, int height, RequestListener * tListener);
	void RetrieveAvatar(std::string username, int width, int height, RequestListener * tListener);
	void Start(Request * request, RequestListener * tLIstener, int identifier = 0);
	
	bool CheckRequestListener(ListenerHandle handle);
	ListenerHandle AttachRequestListener(RequestListener * tListener);
	void DetachRequestListener(RequestListener * tListener);
	enum ProcessResponse { Finished, OK, Canceled, Failed, Duplicate };
	class Request
	{
	public:
		enum RequestType { ThumbnailRender, Image, API, Web };
		int Identifier;
		RequestType Type;
		void * ResultObject;
		ListenerHandle Listener;
		std::vector<Request*> Children;
		Request(RequestType type, ListenerHandle listener, int identifier);
		virtual ProcessResponse Process(RequestBroker & rb) { return Failed; }
		virtual ~Request();
		virtual void Cleanup();
	};
};
