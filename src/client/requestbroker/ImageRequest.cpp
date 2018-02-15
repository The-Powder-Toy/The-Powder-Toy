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
	HTTPContext = nullptr;
	Width = width;
	Height = height;
}

ImageRequest::~ImageRequest()
{

}

RequestBroker::ProcessResponse ImageRequest::Process(RequestBroker & rb)
{
	VideoBuffer * image = nullptr;

	//Have a look at the thumbnail cache
	for(auto & iter : rb.imageCache)
	{
		if(iter.first == URL)
		{
			image = iter.second;
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
			for(auto & activeRequest : rb.activeRequests)
			{
				if(activeRequest->Type != Request::Image)
					continue;
				auto * otherReq = (ImageRequest*)activeRequest;
				if(otherReq->URL == URL && otherReq != this)
				{
/*#ifdef DEBUG
					std::cout << typeid(*this).name() << " Request for " << URL << " found, appending." << std::endl;
#endif*/
					//Add the current listener to the item already being requested
					activeRequest->Children.push_back(this);
					return RequestBroker::Duplicate;
				}
			}

			//If it's not already being requested, request it
/*#ifdef DEBUG
			std::cout << typeid(*this).name() << " Creating new request for " << URL << std::endl;
#endif*/
			HTTPContext = http_async_req_start(nullptr, (char *)URL.c_str(), nullptr, 0, 0);
			RequestTime = time(nullptr);
		}
	}

	if(image)
	{

		//Create a copy, to separate from the cache
		std::vector<Request *> children(Children.begin(), Children.end());
		Children.clear();

		auto * myVB = new VideoBuffer(*image);
		myVB->Resize(Width, Height, true);
		ResultObject = (void*)myVB;
		rb.requestComplete(this);
		for(auto & childIter : children)
		{
			if(childIter->Type == Request::Image)
			{
				auto * childReq = (ImageRequest*)childIter;
				auto * tempImage = new VideoBuffer(*image);
				tempImage->Resize(childReq->Width, childReq->Height, true);
				childReq->ResultObject = (void*)tempImage;
				rb.requestComplete(childIter);
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
		ResultObject = nullptr;
	}
}
