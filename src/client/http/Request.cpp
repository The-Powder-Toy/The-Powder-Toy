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
	downloadData(NULL),
	downloadSize(0),
	downloadStatus(0),
	postData(""),
	postDataBoundary(""),
	userID(""),
	userSession(""),
	downloadFinished(false),
	downloadCanceled(false),
	downloadStarted(false)
{
	uri = ByteString(uri_);
	RequestManager::Ref().AddDownload(this);
}

// called by download thread itself if download was canceled
Request::~Request()
{
	if (http && (keepAlive || downloadCanceled))
		http_async_req_close(http);
	if (downloadData)
		free(downloadData);
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

// add userID and sessionID headers to the download. Must be done after download starts for some reason
void Request::AuthHeaders(ByteString ID, ByteString session)
{
	if (ID != "0")
		userID = ID;
	userSession = session;
}

// start the download thread
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
	downloadStarted = true;
	RequestManager::Ref().Unlock();
}


// finish the download (if called before the download is done, this will block)
ByteString Request::Finish(int *status)
{
	if (CheckCanceled())
		return ""; // shouldn't happen but just in case
	while (!CheckDone()); // block
	RequestManager::Ref().Lock();
	downloadStarted = false;
	if (status)
		*status = downloadStatus;
	ByteString ret;
	if (downloadData)
	{
		ret = ByteString(downloadData, downloadData + downloadSize);
		free(downloadData);
	}
	downloadData = NULL;
	if (!keepAlive)
		downloadCanceled = true;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns the download size and progress (if the download has the correct length headers)
void Request::CheckProgress(int *total, int *done)
{
	RequestManager::Ref().Lock();
	if (!downloadFinished && http)
		http_async_get_length(http, total, done);
	else
		*total = *done = 0;
	RequestManager::Ref().Unlock();
}

// returns true if the download has finished
bool Request::CheckDone()
{
	RequestManager::Ref().Lock();
	bool ret = downloadFinished;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns true if the download was canceled
bool Request::CheckCanceled()
{
	RequestManager::Ref().Lock();
	bool ret = downloadCanceled;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns true if the download is running
bool Request::CheckStarted()
{
	RequestManager::Ref().Lock();
	bool ret = downloadStarted;
	RequestManager::Ref().Unlock();
	return ret;

}

// cancels the download, the download thread will delete the Request* when it finishes (do not use Request in any way after canceling)
void Request::Cancel()
{
	RequestManager::Ref().Lock();
	downloadCanceled = true;
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

