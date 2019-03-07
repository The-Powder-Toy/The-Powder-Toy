#include "RequestManager.h"
#include "Request.h"
#include "HTTP.h"
#include "Config.h"
#include "Platform.h"

namespace http
{
RequestManager::RequestManager():
	threadStarted(false),
	lastUsed(time(NULL)),
	managerRunning(false),
	managerShutdown(false),
	requests(std::vector<Request*>()),
	requestsAddQueue(std::vector<Request*>())
{
	pthread_mutex_init(&requestLock, NULL);
	pthread_mutex_init(&requestAddLock, NULL);
}

RequestManager::~RequestManager()
{

}

void RequestManager::Shutdown()
{
	pthread_mutex_lock(&requestLock);
	pthread_mutex_lock(&requestAddLock);
	for (std::vector<Request*>::iterator iter = requests.begin(); iter != requests.end(); ++iter)
	{
		Request *request = (*iter);
		if (request->http)
			http_force_close(request->http);
		request->requestCanceled = true;
		delete request;
	}
	requests.clear();
	requestsAddQueue.clear();
	managerShutdown = true;
	pthread_mutex_unlock(&requestAddLock);
	pthread_mutex_unlock(&requestLock);
	if (threadStarted)
		pthread_join(requestThread, NULL);
	
	http_done();
}

//helper function for request
TH_ENTRY_POINT void* RequestManagerHelper(void* obj)
{
	RequestManager *temp = (RequestManager*)obj;
	temp->Update();
	return NULL;
}

void RequestManager::Initialise(ByteString Proxy)
{
	proxy = Proxy;
	if (proxy.length())
	{
		http_init((char *)proxy.c_str());
	}
	else
	{
		http_init(NULL);
	}
}

void RequestManager::Start()
{
	managerRunning = true;
	lastUsed = time(NULL);
	pthread_create(&requestThread, NULL, &RequestManagerHelper, this);
}

void RequestManager::Update()
{
	unsigned int numActiveRequests = 0;
	while (!managerShutdown)
	{
		pthread_mutex_lock(&requestAddLock);
		if (requestsAddQueue.size())
		{
			for (size_t i = 0; i < requestsAddQueue.size(); i++)
			{
				requests.push_back(requestsAddQueue[i]);
			}
			requestsAddQueue.clear();
		}
		pthread_mutex_unlock(&requestAddLock);
		if (requests.size())
		{
			numActiveRequests = 0;
			pthread_mutex_lock(&requestLock);
			for (size_t i = 0; i < requests.size(); i++)
			{
				Request *request = requests[i];
				if (request->requestCanceled)
				{
					if (request->http && request->requestStarted)
						http_force_close(request->http);
					delete request;
					requests.erase(requests.begin()+i);
					i--;
				}
				else if (request->requestStarted && !request->requestFinished)
				{
					if (http_async_req_status(request->http) != 0)
					{
						request->requestData = http_async_req_stop(request->http, &request->requestStatus, &request->requestSize);
						request->requestFinished = true;
						if (!request->keepAlive)
							request->http = NULL;
					}
					lastUsed = time(NULL);
					numActiveRequests++;
				}
			}
			pthread_mutex_unlock(&requestLock);
		}
		if (time(NULL) > lastUsed+http_timeout*2 && !numActiveRequests)
		{
			pthread_mutex_lock(&requestLock);
			managerRunning = false;
			pthread_mutex_unlock(&requestLock);
			return;
		}
		Platform::Millisleep(1);
	}
}

void RequestManager::EnsureRunning()
{
	pthread_mutex_lock(&requestLock);
	if (!managerRunning)
	{
		if (threadStarted)
			pthread_join(requestThread, NULL);
		else
			threadStarted = true;
		Start();
	}
	pthread_mutex_unlock(&requestLock);
}

void RequestManager::AddRequest(Request *request)
{
	pthread_mutex_lock(&requestAddLock);
	requestsAddQueue.push_back(request);
	pthread_mutex_unlock(&requestAddLock);
	EnsureRunning();
}

void RequestManager::Lock()
{
	pthread_mutex_lock(&requestAddLock);
}

void RequestManager::Unlock()
{
	pthread_mutex_unlock(&requestAddLock);
}
}
