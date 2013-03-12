#pragma once
#include <queue>
#include <list>
#include <utility>
#include <deque>
#include <pthread.h>
#undef GetUserName //God dammit microsoft!

#include "Singleton.h"

class GameSave;
class VideoBuffer;
class RequestListener;
typedef std::pair<int, RequestListener*> ListenerHandle;
class RequestBroker: public Singleton<RequestBroker>
{
private: 

	enum ProcessResponse { Finished, OK, Canceled, Failed, Duplicate };

	class Request
	{
	public:
		enum RequestType { ThumbnailRender, Image };
		RequestType Type;
		void * ResultObject;
		ListenerHandle Listener;
		std::vector<Request*> Children;
		Request(RequestType type, ListenerHandle listener)
		{
			Type = type;
			Listener = listener;
			ResultObject = NULL;
		}
		virtual ~Request()
		{
			std::vector<Request*>::iterator iter = Children.begin();
			while(iter != Children.end())
			{
				delete (*iter);
				iter++;
			}
		}
		virtual void Cleanup()
		{
			std::vector<Request*>::iterator iter = Children.begin();
			while(iter != Children.end())
			{
				(*iter)->Cleanup();
				iter++;
			}
		}
	};

	class ThumbRenderRequest: public Request
	{
	public:
		int Width, Height;
		bool Decorations;
		bool Fire;
		GameSave * Save;
		ThumbRenderRequest(GameSave * save, bool decorations, bool fire, int width, int height, ListenerHandle listener):
			Request(ThumbnailRender, listener)
		{
			Save = save;
			Width = width;
			Height = height;
			Decorations = decorations;
			Fire = fire;
		}
		virtual ~ThumbRenderRequest()
		{
			if(Save)
				delete Save;
		}
		virtual void Cleanup()
		{
			Request::Cleanup();
			if(ResultObject)
			{
				delete ((VideoBuffer*)ResultObject);
				ResultObject = NULL;
			}
		}
	};

	class ImageRequest: public Request
	{
	public:
		int Width, Height;
		std::string URL;
		int RequestTime;
		void * HTTPContext;
		ImageRequest(std::string url, int width, int height, ListenerHandle listener):
			Request(Image, listener)
		{
			URL = url;
			HTTPContext = NULL;
			Width = width;
			Height = height;
		}
		virtual ~ImageRequest() {}
		virtual void Cleanup()
		{
			Request::Cleanup();
			if(ResultObject)
			{
				delete ((VideoBuffer*)ResultObject);
				ResultObject = NULL;
			}
		}
	};

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

	static void * thumbnailQueueProcessHelper(void * ref);
	void thumbnailQueueProcessTH();
	void assureRunning();

	ProcessResponse processThumbnailRender(ThumbRenderRequest & request);
	ProcessResponse processImage(ImageRequest & request);

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
	
	bool CheckRequestListener(ListenerHandle handle);
	ListenerHandle AttachRequestListener(RequestListener * tListener);
	void DetachRequestListener(RequestListener * tListener);
};