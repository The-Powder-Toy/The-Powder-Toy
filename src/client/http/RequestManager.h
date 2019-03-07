#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H
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
	pthread_t requestThread;
	pthread_mutex_t requestLock;
	pthread_mutex_t requestAddLock;
	bool threadStarted;
	ByteString proxy;

	int lastUsed;
	volatile bool managerRunning;
	volatile bool managerShutdown;
	std::vector<Request*> requests;
	std::vector<Request*> requestsAddQueue;

	void Start();
public:
	RequestManager();
	~RequestManager();

	void Initialise(ByteString proxy);

	void Shutdown();
	void Update();
	void EnsureRunning();

	void AddRequest(Request *request);
	void RemoveRequest(int id);

	void Lock();
	void Unlock();
};
}

#endif // REQUESTMANAGER_H
