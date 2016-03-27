#ifndef DOWNLOAD_H
#define DOWNLOAD_H
#include <map>
#include <string>

class DownloadManager;
class Download
{
	std::string uri;
	void *http;
	bool keepAlive;

	char *downloadData;
	int downloadSize;
	int downloadStatus;

	std::string postData;
	std::string postDataBoundary;

	std::string userID;
	std::string userSession;

	volatile bool downloadFinished;
	volatile bool downloadCanceled;
	volatile bool downloadStarted;

public:
	Download(std::string uri, bool keepAlive = false);
	~Download();

	void AddPostData(std::map<std::string, std::string> data);
	void AddPostData(std::pair<std::string, std::string> data);
	void AuthHeaders(std::string ID, std::string session);
	void Start();
	bool Reuse(std::string newuri);
	char* Finish(int *length, int *status);
	void Cancel();

	void CheckProgress(int *total, int *done);
	bool CheckDone();
	bool CheckCanceled();
	bool CheckStarted();

	friend class DownloadManager;
};

#endif
