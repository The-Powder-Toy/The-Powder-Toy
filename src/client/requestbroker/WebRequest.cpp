#include <iostream>
#include <vector>
#include <typeinfo>
#include <cstdlib>
#include <cstring>
#include "Config.h"
#include "Format.h"
#include "client/Client.h"
#include "WebRequest.h"
#include "client/HTTP.h"
#include "APIResultParser.h"

WebRequest::WebRequest(std::string url, ListenerHandle listener, int identifier):
	RequestBroker::Request(API, listener, identifier)
{
	Post = false;
	HTTPContext = NULL;
	URL = url;
}

WebRequest::WebRequest(std::string url, std::map<std::string, std::string> postData, ListenerHandle listener, int identifier):
	RequestBroker::Request(API, listener, identifier)
{
	Post = true;
	PostData = postData;
	HTTPContext = NULL;
	URL = url;
}

RequestBroker::ProcessResponse WebRequest::Process(RequestBroker & rb)
{
	if(HTTPContext)
	{
		if(http_async_req_status(HTTPContext))
		{
			char * data;
			int status, data_size;
			data = http_async_req_stop(HTTPContext, &status, &data_size);

			Client::Ref().ParseServerReturn(NULL, status, true);
			if (status == 200 && data)
			{
				void * resultObject = new std::vector<unsigned char>(data, data+data_size);

				if(resultObject)
				{
					this->ResultObject = resultObject;
					rb.requestComplete(this);
					free(data);
					return RequestBroker::Finished;
				}
				else
				{
#ifdef DEBUG
					std::cout << typeid(*this).name() << " Request for " << URL << " could not be parsed: " << data << std::endl;
#endif
					free(data);
					return RequestBroker::Failed;
				}
			}
			else
			{
#ifdef DEBUG
				std::cout << typeid(*this).name() << " Request for " << URL << " failed with status " << status << std::endl;
#endif
				free(data);

				return RequestBroker::Failed;
			}
		}
	}
	else 
	{
#ifdef DEBUG
		std::cout << typeid(*this).name() << " New Request for " << URL << std::endl;
#endif
		if(Post)
		{
			char ** postNames = new char*[PostData.size() + 1];
			char ** postData = new char*[PostData.size()];
			int * postLength = new int[PostData.size()];

			int i = 0;
			std::map<std::string, std::string>::iterator iter = PostData.begin();
			while(iter != PostData.end())
			{
				std::string name = iter->first;
				std::string data = iter->second;
				char * cName = new char[name.length() + 1];
				char * cData = new char[data.length() + 1];
				std::strcpy(cName, name.c_str());
				std::strcpy(cData, data.c_str());
				postNames[i] = cName;
				postData[i] = cData;
				postLength[i] = data.length();
				i++;
				iter++;
			}
			postNames[i] = NULL;

			if(Client::Ref().GetAuthUser().UserID)
			{
#ifdef DEBUG
				std::cout << typeid(*this).name() << " Authenticated " << std::endl;
#endif
				User user = Client::Ref().GetAuthUser();
				char userName[12];
				char *userSession = new char[user.SessionID.length() + 1];
				std::strcpy(userName, format::NumberToString<int>(user.UserID).c_str());
				std::strcpy(userSession, user.SessionID.c_str());
				HTTPContext = http_multipart_post_async((char*)URL.c_str(), postNames, postData, postLength, userName, NULL, userSession);
				delete[] userSession;
			}
			else
			{
				HTTPContext = http_multipart_post_async((char*)URL.c_str(), postNames, postData, postLength, NULL, NULL, NULL);
			}

		}
		else
		{
			HTTPContext = http_async_req_start(NULL, (char *)URL.c_str(), NULL, 0, 0);
			if(Client::Ref().GetAuthUser().UserID)
			{
				User user = Client::Ref().GetAuthUser();
				char userName[12];
				char *userSession = new char[user.SessionID.length() + 1];
				std::strcpy(userName, format::NumberToString<int>(user.UserID).c_str());
				std::strcpy(userSession, user.SessionID.c_str());
				http_auth_headers(HTTPContext, userName, NULL, userSession);
				delete[] userSession;
			}
		}
	}
	return RequestBroker::OK;
}

WebRequest::~WebRequest()
{
}

void WebRequest::Cleanup()
{
	Request::Cleanup();
	if(ResultObject)
	{
		delete (std::vector<unsigned char>*)ResultObject;
		ResultObject = NULL;
	}
}
