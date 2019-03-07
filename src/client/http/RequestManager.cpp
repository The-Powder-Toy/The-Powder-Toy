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
	downloads(std::vector<Request*>()),
	downloadsAddQueue(std::vector<Request*>())
{
	pthread_mutex_init(&downloadLock, NULL);
	pthread_mutex_init(&downloadAddLock, NULL);
}

RequestManager::~RequestManager()
{

}

void RequestManager::Shutdown()
{
	pthread_mutex_lock(&downloadLock);
	pthread_mutex_lock(&downloadAddLock);
	for (std::vector<Request*>::iterator iter = downloads.begin(); iter != downloads.end(); ++iter)
	{
		Request *download = (*iter);
		if (download->http)
			http_force_close(download->http);
		download->downloadCanceled = true;
		delete download;
	}
	downloads.clear();
	downloadsAddQueue.clear();
	managerShutdown = true;
	pthread_mutex_unlock(&downloadAddLock);
	pthread_mutex_unlock(&downloadLock);
	if (threadStarted)
		pthread_join(downloadThread, NULL);
	
	http_done();
}

//helper function for download
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
	pthread_create(&downloadThread, NULL, &RequestManagerHelper, this);
}

void RequestManager::Update()
{
	unsigned int numActiveDownloads = 0;
	while (!managerShutdown)
	{
		pthread_mutex_lock(&downloadAddLock);
		if (downloadsAddQueue.size())
		{
			for (size_t i = 0; i < downloadsAddQueue.size(); i++)
			{
				downloads.push_back(downloadsAddQueue[i]);
			}
			downloadsAddQueue.clear();
		}
		pthread_mutex_unlock(&downloadAddLock);
		if (downloads.size())
		{
			numActiveDownloads = 0;
			pthread_mutex_lock(&downloadLock);
			for (size_t i = 0; i < downloads.size(); i++)
			{
				Request *download = downloads[i];
				if (download->downloadCanceled)
				{
					if (download->http && download->downloadStarted)
						http_force_close(download->http);
					delete download;
					downloads.erase(downloads.begin()+i);
					i--;
				}
				else if (download->downloadStarted && !download->downloadFinished)
				{
					if (http_async_req_status(download->http) != 0)
					{
						download->downloadData = http_async_req_stop(download->http, &download->downloadStatus, &download->downloadSize);
						download->downloadFinished = true;
						if (!download->keepAlive)
							download->http = NULL;
					}
					lastUsed = time(NULL);
					numActiveDownloads++;
				}
			}
			pthread_mutex_unlock(&downloadLock);
		}
		if (time(NULL) > lastUsed+http_timeout*2 && !numActiveDownloads)
		{
			pthread_mutex_lock(&downloadLock);
			managerRunning = false;
			pthread_mutex_unlock(&downloadLock);
			return;
		}
		Platform::Millisleep(1);
	}
}

void RequestManager::EnsureRunning()
{
	pthread_mutex_lock(&downloadLock);
	if (!managerRunning)
	{
		if (threadStarted)
			pthread_join(downloadThread, NULL);
		else
			threadStarted = true;
		Start();
	}
	pthread_mutex_unlock(&downloadLock);
}

void RequestManager::AddDownload(Request *download)
{
	pthread_mutex_lock(&downloadAddLock);
	downloadsAddQueue.push_back(download);
	pthread_mutex_unlock(&downloadAddLock);
	EnsureRunning();
}

void RequestManager::Lock()
{
	pthread_mutex_lock(&downloadAddLock);
}

void RequestManager::Unlock()
{
	pthread_mutex_unlock(&downloadAddLock);
}
}
