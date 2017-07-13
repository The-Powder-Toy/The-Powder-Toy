#include <iostream>
#include <typeinfo>
#include <ctime>
#include "ImageRequest.h"
#include "graphics/Graphics.h"
#include "client/HTTP.h"

ImageRequest::ImageRequest(std::string url, int width, int height, ListenerHandle listener, int identifier):
	Request(Image, listener, identifier)
{
	URL = url;
	HTTPContext = NULL;
	Width = width;
	Height = height;
}

ImageRequest::~ImageRequest()
{

}

RequestBroker::ProcessResponse ImageRequest::Process(RequestBroker & rb)
{
	VideoBuffer * image = NULL;

	//Have a look at the thumbnail cache
	for(std::deque<std::pair<std::string, VideoBuffer*> >::iterator iter = rb.imageCache.begin(), end = rb.imageCache.end(); iter != end; ++iter)
	{
		if((*iter).first == URL)
		{
			image = (*iter).second;
/*#ifdef DEBUG
			std::cout << typeid(*this).name() << " " << URL << " found in cache" << std::endl;
#endif*/
		}
	}

	if(!image)
	{
		if(HTTPContext)
		{
			if(http_async_req_status(HTTPContext))
			{
				pixel * imageData;
				char * data;
				int status, data_size, imgw, imgh;
				data = http_async_req_stop(HTTPContext, &status, &data_size);

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

					if(rb.imageCache.size() >= THUMB_CACHE_SIZE)
					{
						//Remove unnecessary from thumbnail cache
						delete rb.imageCache.front().second;
						rb.imageCache.pop_front();
					}
					rb.imageCache.push_back(std::pair<std::string, VideoBuffer*>(URL, image));
				}
				else
				{
#ifdef DEBUG
					std::cout << typeid(*this).name() << " Request for " << URL << " failed with status " << status << std::endl;
#endif	
					free(data);

					return RequestBroker::Failed;
				}
			}
		}
		else 
		{
			//Check for ongoing requests
			for(std::vector<Request*>::iterator iter = rb.activeRequests.begin(), end = rb.activeRequests.end(); iter != end; ++iter)
			{
				if((*iter)->Type != Request::Image)
					continue;
				ImageRequest * otherReq = (ImageRequest*)(*iter);
				if(otherReq->URL == URL && otherReq != this)
				{
/*#ifdef DEBUG
					std::cout << typeid(*this).name() << " Request for " << URL << " found, appending." << std::endl;
#endif*/
					//Add the current listener to the item already being requested
					(*iter)->Children.push_back(this);
					return RequestBroker::Duplicate;
				}
			}

			//If it's not already being requested, request it
/*#ifdef DEBUG
			std::cout << typeid(*this).name() << " Creating new request for " << URL << std::endl;
#endif*/
			HTTPContext = http_async_req_start(NULL, (char *)URL.c_str(), NULL, 0, 0);
			RequestTime = time(NULL);
		}
	}
	
	if(image)
	{

		//Create a copy, to separate from the cache
		std::vector<Request *> children(Children.begin(), Children.end());
		Children.clear();

		VideoBuffer * myVB = new VideoBuffer(*image);
		myVB->Resize(Width, Height, true);
		ResultObject = (void*)myVB;
		rb.requestComplete(this);
		for(std::vector<Request*>::iterator childIter = children.begin(), childEnd = children.end(); childIter != childEnd; ++childIter)
		{
			if((*childIter)->Type == Request::Image)
			{
				ImageRequest * childReq = (ImageRequest*)*childIter;
				VideoBuffer * tempImage = new VideoBuffer(*image);
				tempImage->Resize(childReq->Width, childReq->Height, true);
				childReq->ResultObject = (void*)tempImage;
				rb.requestComplete(*childIter);
			}
		}
		return RequestBroker::Finished;
	}

	return RequestBroker::OK;
}

void ImageRequest::Cleanup()
{
	Request::Cleanup();
	if(ResultObject)
	{
		delete ((VideoBuffer*)ResultObject);
		ResultObject = NULL;
	}
}
