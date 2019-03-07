#include <cstdlib>
#include "Request.h"
#include "RequestManager.h"
#include "HTTP.h"
#include "Platform.h"

namespace http
{
Request::Request(ByteString uri_, bool keepAlive):
	http(NULL),
	keepAlive(keepAlive),
	requestData(NULL),
	requestSize(0),
	requestStatus(0),
	postData(""),
	postDataBoundary(""),
	userID(""),
	userSession(""),
	requestFinished(false),
	requestCanceled(false),
	requestStarted(false)
{
	uri = ByteString(uri_);
	RequestManager::Ref().AddRequest(this);
}

// called by request thread itself if request was canceled
Request::~Request()
{
	if (http && (keepAlive || requestCanceled))
		http_async_req_close(http);
	if (requestData)
		free(requestData);
}

// add post data to a request
void Request::AddPostData(std::map<ByteString, ByteString> data)
{
	postDataBoundary = FindBoundary(data, "");
	postData = GetMultipartMessage(data, postDataBoundary);
}
void Request::AddPostData(std::pair<ByteString, ByteString> data)
{
	std::map<ByteString, ByteString> postData;
	postData.insert(data);
	AddPostData(postData);
}

// add userID and sessionID headers to the request. Must be done after request starts for some reason
void Request::AuthHeaders(ByteString ID, ByteString session)
{
	if (ID != "0")
		userID = ID;
	userSession = session;
}

// start the request thread
void Request::Start()
{
	if (CheckStarted() || CheckDone())
		return;
	http = http_async_req_start(http, uri.c_str(), postData.c_str(), postData.length(), keepAlive ? 1 : 0);
	// add the necessary headers
	if (userID.length() || userSession.length())
		http_auth_headers(http, userID.c_str(), NULL, userSession.c_str());
	if (postDataBoundary.length())
		http_add_multipart_header(http, postDataBoundary);
	RequestManager::Ref().Lock();
	requestStarted = true;
	RequestManager::Ref().Unlock();
}


// finish the request (if called before the request is done, this will block)
ByteString Request::Finish(int *status)
{
	if (CheckCanceled())
		return ""; // shouldn't happen but just in case
	while (!CheckDone()); // block
	RequestManager::Ref().Lock();
	requestStarted = false;
	if (status)
		*status = requestStatus;
	ByteString ret;
	if (requestData)
	{
		ret = ByteString(requestData, requestData + requestSize);
		free(requestData);
	}
	requestData = NULL;
	if (!keepAlive)
		requestCanceled = true;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns the request size and progress (if the request has the correct length headers)
void Request::CheckProgress(int *total, int *done)
{
	RequestManager::Ref().Lock();
	if (!requestFinished && http)
		http_async_get_length(http, total, done);
	else
		*total = *done = 0;
	RequestManager::Ref().Unlock();
}

// returns true if the request has finished
bool Request::CheckDone()
{
	RequestManager::Ref().Lock();
	bool ret = requestFinished;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns true if the request was canceled
bool Request::CheckCanceled()
{
	RequestManager::Ref().Lock();
	bool ret = requestCanceled;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns true if the request is running
bool Request::CheckStarted()
{
	RequestManager::Ref().Lock();
	bool ret = requestStarted;
	RequestManager::Ref().Unlock();
	return ret;

}

// cancels the request, the request thread will delete the Request* when it finishes (do not use Request in any way after canceling)
void Request::Cancel()
{
	RequestManager::Ref().Lock();
	requestCanceled = true;
	RequestManager::Ref().Unlock();
}

ByteString Request::Simple(ByteString uri, int *status, std::map<ByteString, ByteString> post_data)
{
	Request *request = new Request(uri);
	request->AddPostData(post_data);
	request->Start();
	while(!request->CheckDone())
	{
		Platform::Millisleep(1);
	}
	return request->Finish(status);
}

ByteString Request::SimpleAuth(ByteString uri, int *status, ByteString ID, ByteString session, std::map<ByteString, ByteString> post_data)
{
	Request *request = new Request(uri);
	request->AddPostData(post_data);
	request->AuthHeaders(ID, session);
	request->Start();
	while(!request->CheckDone())
	{
		Platform::Millisleep(1);
	}
	return request->Finish(status);
}

const char *StatusText(int code)
{
	return http_ret_text(code);
}
}

