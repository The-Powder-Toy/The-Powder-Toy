#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <time.h>
#include "RequestBroker.h"
#include "RequestListener.h"
#include "Client.h"
#include "HTTP.h"
#include "GameSave.h"
#include "search/Thumbnail.h"
#include "simulation/SaveRenderer.h"

//Asynchronous Thumbnail render & request processing

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

void RequestBroker::RetrieveImage(std::string imageUrl, int width, int height, RequestListener * tListener)
{
	ListenerHandle handle = AttachRequestListener(tListener);

	ImageRequest * r = new ImageRequest(imageUrl, width, height, handle);
	
	pthread_mutex_lock(&requestQueueMutex);
	requestQueue.push_back(r);
	pthread_mutex_unlock(&requestQueueMutex);

	assureRunning();
}

void * RequestBroker::thumbnailQueueProcessHelper(void * ref)
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
			completeQueue.front()->Listener.second->OnResponseReady(completeQueue.front()->ResultObject);
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
				switch(r->Type)
				{
				case Request::ThumbnailRender:
					resultStatus = processThumbnailRender(*(ThumbRenderRequest*)r);
					break;
				case Request::Image:
					resultStatus = processImage(*(ImageRequest*)r);
					break;
				}
				if(resultStatus == Duplicate || resultStatus == Failed || resultStatus == Finished)
				{
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
	}
	pthread_mutex_lock(&runningMutex);
	thumbnailQueueRunning = false;
	pthread_mutex_unlock(&runningMutex);
}

RequestBroker::ProcessResponse RequestBroker::processThumbnailRender(ThumbRenderRequest & request)
{
#ifdef DEBUG
		std::cout << typeid(*this).name() << " Processing render request" << std::endl;
#endif
	Thumbnail * thumbnail = SaveRenderer::Ref().Render(request.Save, request.Decorations, request.Fire);
	delete request.Save;
	request.Save = NULL;

	if(thumbnail)
	{
		thumbnail->Resize(request.Width, request.Height);
		request.ResultObject = (void*)thumbnail;
		requestComplete(&request);
		return Finished;
	}
	else
	{
		return Failed;
	}
	return Failed;
}

RequestBroker::ProcessResponse RequestBroker::processImage(ImageRequest & request)
{
	VideoBuffer * image = NULL;

	//Have a look at the thumbnail cache
	for(std::deque<std::pair<std::string, VideoBuffer*> >::iterator iter = imageCache.begin(), end = imageCache.end(); iter != end; ++iter)
	{
		if((*iter).first == request.URL)
		{
			image = (*iter).second;
#ifdef DEBUG
			std::cout << typeid(*this).name() << " " << request.URL << " found in cache" << std::endl;
#endif
		}
	}

	if(!image)
	{
		if(request.HTTPContext)
		{
			if(http_async_req_status(request.HTTPContext))
			{
				pixel * imageData;
				char * data;
				int status, data_size, imgw, imgh;
				data = http_async_req_stop(request.HTTPContext, &status, &data_size);

				if (status == 200 && data)
				{
					imageData = Graphics::ptif_unpack(data, data_size, &imgw, &imgh);
					free(data);

					if(imageData)
					{
						//Success!
						image = new VideoBuffer(imageData, imgw, imgh);
						free(imageData);
					}
					else
					{
						//Error thumbnail
						image = new VideoBuffer(32, 32);
						image->SetCharacter(14, 14, 'x', 255, 255, 255, 255);
					}

					if(imageCache.size() >= THUMB_CACHE_SIZE)
					{
						//Remove unnecessary from thumbnail cache
						delete imageCache.front().second;
						imageCache.pop_front();
					}
					imageCache.push_back(std::pair<std::string, VideoBuffer*>(request.URL, image));
				}
				else
				{
	#ifdef DEBUG
					std::cout << typeid(*this).name() << " Request for " << request.URL << " failed with status " << status << std::endl;
	#endif	
					if(data)
						free(data);

					return Failed;
				}
			}
		}
		else 
		{
			//Check for ongoing requests
			for(std::vector<Request*>::iterator iter = activeRequests.begin(), end = activeRequests.end(); iter != end; ++iter)
			{
				if((*iter)->Type != Request::Image)
					continue;
				ImageRequest * otherReq = (ImageRequest*)(*iter);
				if(otherReq->URL == request.URL && otherReq != &request)
				{
	#ifdef DEBUG
					std::cout << typeid(*this).name() << " Request for " << request.URL << " found, appending." << std::endl;
	#endif
					//Add the current listener to the item already being requested
					(*iter)->Children.push_back(&request);
					return Duplicate;
				}
			}

			//If it's not already being requested, request it
	#ifdef DEBUG
			std::cout << typeid(*this).name() << " Creating new request for " << request.URL << std::endl;
	#endif
			request.HTTPContext = http_async_req_start(NULL, (char *)request.URL.c_str(), NULL, 0, 0);
			request.RequestTime = time(NULL);
		}
	}
	
	if(image)
	{

		//Create a copy, to seperate from the cache
		VideoBuffer * myVB = new VideoBuffer(*image);
		myVB->Resize(request.Width, request.Height, true);
		request.ResultObject = (void*)myVB;
		requestComplete(&request);
		for(std::vector<Request*>::iterator childIter = request.Children.begin(), childEnd = request.Children.end(); childIter != childEnd; ++childIter)
		{
			if((*childIter)->Type == Request::Image)
			{
				ImageRequest * childReq = (ImageRequest*)*childIter;
				VideoBuffer * tempImage = new VideoBuffer(*image);
				tempImage->Resize(childReq->Width, childReq->Height, true);
				childReq->ResultObject = (void*)tempImage;
				requestComplete(*childIter);
			}
		}
		return Finished;
	}

	return OK;
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
	ListenerHandle handle = ListenerHandle(tListener->ListenerRand, tListener);
	pthread_mutex_lock(&listenersMutex);
	validListeners.push_back(handle);
	pthread_mutex_unlock(&listenersMutex);
	return handle;
}

void RequestBroker::DetachRequestListener(RequestListener * tListener)
{
	pthread_mutex_lock(&listenersMutex);

	std::vector<ListenerHandle>::iterator iter = validListeners.begin();
	while (iter != validListeners.end())
	{
		if(*iter == ListenerHandle(tListener->ListenerRand, tListener))
			iter = validListeners.erase(iter);
		else
			++iter;
	}

	pthread_mutex_unlock(&listenersMutex);
}