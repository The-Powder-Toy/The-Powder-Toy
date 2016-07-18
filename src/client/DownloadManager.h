#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H
#include "common/tpt-thread.h"
#include <ctime>
#include <vector>
#include "common/Singleton.h"

class Download;
class DownloadManager : public Singleton<DownloadManager>
{
private:
	pthread_t downloadThread;
	pthread_mutex_t downloadLock;
	pthread_mutex_t downloadAddLock;
	bool threadStarted;

	int lastUsed;
	volatile bool managerRunning;
	volatile bool managerShutdown;
	std::vector<Download*> downloads;
	std::vector<Download*> downloadsAddQueue;

	void Start();
public:
	DownloadManager();
	~DownloadManager();

	void Shutdown();
	void Update();
	void EnsureRunning();

	void AddDownload(Download *download);
	void RemoveDownload(int id);

	void Lock();
	void Unlock();
};

#endif // DOWNLOADMANAGER_H
