#ifndef REQUEST_H
#define REQUEST_H
#include <map>
#include "common/String.h"

namespace http
{
class RequestManager;
class Request
{
	ByteString uri;
	void *http;
	bool keepAlive;

	char *requestData;
	int requestSize;
	int requestStatus;

	ByteString postData;
	ByteString postDataBoundary;

	ByteString userID;
	ByteString userSession;

	volatile bool requestFinished;
	volatile bool requestCanceled;
	volatile bool requestStarted;

public:
	Request(ByteString uri, bool keepAlive = false);
	virtual ~Request();

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

	friend class RequestManager;

	static ByteString Simple(ByteString uri, int *status, std::map<ByteString, ByteString> post_data = std::map<ByteString, ByteString>{});
	static ByteString SimpleAuth(ByteString uri, int *status, ByteString ID, ByteString session, std::map<ByteString, ByteString> post_data = std::map<ByteString, ByteString>{});
};

const char *StatusText(int code);
}

#endif
