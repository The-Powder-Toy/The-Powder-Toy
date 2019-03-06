#ifndef DOWNLOAD_H
#define DOWNLOAD_H
#include <map>
#include "common/String.h"

namespace http
{
class DownloadManager;
class Download
{
	ByteString uri;
	void *http;
	bool keepAlive;

	char *downloadData;
	int downloadSize;
	int downloadStatus;

	ByteString postData;
	ByteString postDataBoundary;

	ByteString userID;
	ByteString userSession;

	volatile bool downloadFinished;
	volatile bool downloadCanceled;
	volatile bool downloadStarted;

public:
	Download(ByteString uri, bool keepAlive = false);
	virtual ~Download();

	void AddPostData(std::map<ByteString, ByteString> data);
	void AddPostData(std::pair<ByteString, ByteString> data);
	void AuthHeaders(ByteString ID, ByteString session);
	void Start();
	ByteString Finish(int *status);
	void Cancel();

	void CheckProgress(int *total, int *done);
	bool CheckDone();
	bool CheckCanceled();
	bool CheckStarted();

	friend class DownloadManager;

	static ByteString Simple(ByteString uri, int *status, std::map<ByteString, ByteString> post_data = {});
	static ByteString SimpleAuth(ByteString uri, int *status, ByteString ID, ByteString session, std::map<ByteString, ByteString> post_data = {});
};

const char *StatusText(int code);
}

#endif
