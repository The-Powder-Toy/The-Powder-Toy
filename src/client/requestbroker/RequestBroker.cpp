#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <ctime>
#include "RequestBroker.h"
#include "RequestListener.h"
#include "ThumbRenderRequest.h"
#include "ImageRequest.h"
#include "Platform.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "graphics/Graphics.h"

//Asynchronous Thumbnail render & request processing

unsigned int RequestListener::nextListenerID = 0;

RequestBroker::RequestBroker()
{
	thumbnailQueueRunning = false;

	//listenersMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init (&listenersMutex, NULL);


	pthread_mutex_init (&runningMutex, NULL);


	pthread_mutex_init (&requestQueueMutex, NULL);


	pthread_mutex_init (&completeQueueMutex, NULL);
}

RequestBroker::~RequestBroker()
{
	for(std::deque<std::pair<std::string, VideoBuffer*> >::iterator iter = imageCache.begin(), end = imageCache.end(); iter != end; ++iter)
	{
		delete (*iter).second;
	}
}

void RequestBroker::assureRunning()
{
	pthread_mutex_lock(&runningMutex);
	bool running = thumbnailQueueRunning;
	thumbnailQueueRunning = true;
	pthread_mutex_unlock(&runningMutex);

	if(!running)
	{
#ifdef DEBUG
		std::cout << typeid(*this).name() << " Starting background thread for new " << __FUNCTION__ << " request" << std::endl;
#endif
		pthread_create(&thumbnailQueueThread, 0, &RequestBroker::thumbnailQueueProcessHelper, this);
	}
}

void RequestBroker::Shutdown()
{
	pthread_mutex_lock(&runningMutex);
	if(thumbnailQueueRunning)
	{
		thumbnailQueueRunning = false;
		pthread_mutex_unlock(&runningMutex);
		pthread_join(thumbnailQueueThread, NULL);
	}
	else
		pthread_mutex_unlock(&runningMutex);

	std::vector<Request*>::iterator req = activeRequests.begin();
	while(req != activeRequests.end())
	{
		(*req)->Cleanup();
		delete (*req);
		req++;
	}
}

void RequestBroker::RenderThumbnail(GameSave * gameSave, int width, int height, RequestListener * tListener)
{
	RenderThumbnail(gameSave, true, true, width, height, tListener);
}

void RequestBroker::RenderThumbnail(GameSave * gameSave, bool decorations, bool fire, int width, int height, RequestListener * tListener)
{
	ListenerHandle handle = AttachRequestListener(tListener);
	
	ThumbRenderRequest * r = new ThumbRenderRequest(new GameSave(*gameSave), decorations, fire, width, height, handle);
	
	pthread_mutex_lock(&requestQueueMutex);
	requestQueue.push_back(r);
	pthread_mutex_unlock(&requestQueueMutex);
	
	assureRunning();
}

void RequestBroker::RetrieveThumbnail(int saveID, int saveDate, int width, int height, RequestListener * tListener)
{
	std::stringstream urlStream;	
	urlStream << "http://" << STATICSERVER << "/" << saveID;
	if(saveDate)
	{
		urlStream << "_" << saveDate;
	}
	urlStream << "_small.pti";

	RetrieveImage(urlStream.str(), width, height, tListener);
}

void RequestBroker::RetrieveAvatar(std::string username, int width, int height, RequestListener * tListener)
{
	std::stringstream urlStream;	
	urlStream << "http://" << STATICSERVER << "/avatars/" << username << ".pti";

	RetrieveImage(urlStream.str(), width, height, tListener);
}

void RequestBroker::Start(Request * request, RequestListener * tListener, int identifier)
{
	ListenerHandle handle = AttachRequestListener(tListener);

	request->Identifier = identifier;
	request->Listener = handle;
	pthread_mutex_lock(&requestQueueMutex);
	requestQueue.push_back(request);
	pthread_mutex_unlock(&requestQueueMutex);

	assureRunning();	
}

void RequestBroker::RetrieveImage(std::string imageUrl, int width, int height, RequestListener * tListener)
{
	ListenerHandle handle = AttachRequestListener(tListener);

	ImageRequest * r = new ImageRequest(imageUrl, width, height, handle);
	
	pthread_mutex_lock(&requestQueueMutex);
	requestQueue.push_back(r);
	pthread_mutex_unlock(&requestQueueMutex);

	assureRunning();
}

TH_ENTRY_POINT void * RequestBroker::thumbnailQueueProcessHelper(void * ref)
{
	((RequestBroker*)ref)->thumbnailQueueProcessTH();
	return NULL;
}

void RequestBroker::FlushThumbQueue()
{
	pthread_mutex_lock(&completeQueueMutex);
	while(completeQueue.size())
	{
		if(CheckRequestListener(completeQueue.front()->Listener))
		{
			completeQueue.front()->Listener.second->OnResponseReady(completeQueue.front()->ResultObject, completeQueue.front()->Identifier);
		}
		else
		{
#ifdef DEBUG
			std::cout << typeid(*this).name() << " Listener lost, discarding request" << std::endl;
#endif
			completeQueue.front()->Cleanup();
		}
		delete completeQueue.front();
		completeQueue.pop();
	}	
	pthread_mutex_unlock(&completeQueueMutex);
}

void RequestBroker::thumbnailQueueProcessTH()
{
	time_t lastAction = time(NULL);
	pthread_mutex_lock(&runningMutex);
	thumbnailQueueRunning = true;
	pthread_mutex_unlock(&runningMutex);
	while(true)
	{
		//Shutdown after 2 seconds of idle
		if(time(NULL) - lastAction > 2)
		{
#ifdef DEBUG
			std::cout << typeid(*this).name() << " Idle shutdown" << std::endl;
#endif
			break;
		}


		pthread_mutex_lock(&runningMutex);
		bool running = thumbnailQueueRunning;
		pthread_mutex_unlock(&runningMutex);
		if(!running)
		{
#ifdef DEBUG
			std::cout << typeid(*this).name() << " Requested shutdown" << std::endl;
#endif
			break;
		}

		if(activeRequests.size())
		{
			std::vector<Request*>::iterator req = activeRequests.begin();
			while(req != activeRequests.end())
			{
				ProcessResponse resultStatus = OK;
				Request * r = *req;
				resultStatus = r->Process(*this);
				if(resultStatus == Duplicate || resultStatus == Failed || resultStatus == Finished)
				{
					if ((resultStatus == Duplicate || resultStatus == Failed) && CheckRequestListener(r->Listener))
						r->Listener.second->OnResponseFailed(r->Identifier);
					req = activeRequests.erase(req);
				}
				else
				{
					req++;
				}
			}
			lastAction = time(NULL);
		}

		//Move any items from the request queue to the processing queue
		pthread_mutex_lock(&requestQueueMutex);
		std::vector<Request*>::iterator newReq = requestQueue.begin();
		while(newReq != requestQueue.end())
		{
			if(activeRequests.size() > 5)
			{
				break;
			}
			else 
			{
				activeRequests.push_back(*newReq);
				newReq = requestQueue.erase(newReq);
			}
		}
		pthread_mutex_unlock(&requestQueueMutex);
		Platform::Millisleep(1);
	}
	pthread_mutex_lock(&runningMutex);
	thumbnailQueueRunning = false;
	pthread_mutex_unlock(&runningMutex);
}

void RequestBroker::requestComplete(Request * completedRequest)
{
	pthread_mutex_lock(&completeQueueMutex);
	completeQueue.push(completedRequest);
	pthread_mutex_unlock(&completeQueueMutex);
}


void RequestBroker::RetrieveThumbnail(int saveID, int width, int height, RequestListener * tListener)
{
	RetrieveThumbnail(saveID, 0, width, height, tListener);
}

bool RequestBroker::CheckRequestListener(ListenerHandle handle)
{
	pthread_mutex_lock(&listenersMutex);
	int count = std::count(validListeners.begin(), validListeners.end(), handle);
	pthread_mutex_unlock(&listenersMutex);

	return count;
}

ListenerHandle RequestBroker::AttachRequestListener(RequestListener * tListener)
{
	ListenerHandle handle = ListenerHandle(tListener->ListenerID, tListener);
	pthread_mutex_lock(&listenersMutex);
	validListeners.push_back(handle);
	pthread_mutex_unlock(&listenersMutex);
	return handle;
}

void RequestBroker::DetachRequestListener(RequestListener * tListener)
{
	if (!validListeners.size())
		return;

	pthread_mutex_lock(&listenersMutex);

	std::vector<ListenerHandle>::iterator iter = validListeners.begin();
	while (iter != validListeners.end())
	{
		if(*iter == ListenerHandle(tListener->ListenerID, tListener))
			iter = validListeners.erase(iter);
		else
			++iter;
	}

	pthread_mutex_unlock(&listenersMutex);
}

RequestBroker::Request::Request(RequestType type, ListenerHandle listener, int identifier)
{
	Type = type;
	Listener = listener;
	ResultObject = NULL;
	Identifier = identifier;
}
RequestBroker::Request::~Request()
{
	std::vector<Request*>::iterator iter = Children.begin();
	while(iter != Children.end())
	{
		delete (*iter);
		iter++;
	}
	Children.clear();
}
void RequestBroker::Request::Cleanup()
{
	std::vector<Request*>::iterator iter = Children.begin();
	while(iter != Children.end())
	{
		(*iter)->Cleanup();
		iter++;
	}
}
