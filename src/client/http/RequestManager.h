#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H
#include "common/tpt-thread.h"
#include <ctime>
#include <vector>
#include "common/Singleton.h"
#include "common/String.h"

namespace http
{
class Request;
class RequestManager : public Singleton<RequestManager>
{
private:
	pthread_t downloadThread;
	pthread_mutex_t downloadLock;
	pthread_mutex_t downloadAddLock;
	bool threadStarted;
	ByteString proxy;

	int lastUsed;
	volatile bool managerRunning;
	volatile bool managerShutdown;
	std::vector<Request*> downloads;
	std::vector<Request*> downloadsAddQueue;

	void Start();
public:
	RequestManager();
	~RequestManager();

	void Initialise(ByteString proxy);

	void Shutdown();
	void Update();
	void EnsureRunning();

	void AddDownload(Request *download);
	void RemoveDownload(int id);

	void Lock();
	void Unlock();
};
}

#endif // DOWNLOADMANAGER_H
