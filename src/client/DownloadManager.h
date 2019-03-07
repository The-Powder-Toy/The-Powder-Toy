#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H
#include "common/tpt-thread.h"
#include <ctime>
#include <vector>
#include "common/Singleton.h"
#include "common/String.h"

namespace http
{
class Download;
class DownloadManager : public Singleton<DownloadManager>
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
	std::vector<Download*> downloads;
	std::vector<Download*> downloadsAddQueue;

	void Start();
public:
	DownloadManager();
	~DownloadManager();

	void Initialise(ByteString proxy);

	void Shutdown();
	void Update();
	void EnsureRunning();

	void AddDownload(Download *download);
	void RemoveDownload(int id);

	void Lock();
	void Unlock();
};
}

#endif // DOWNLOADMANAGER_H
