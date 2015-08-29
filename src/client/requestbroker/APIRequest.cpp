#include <iostream>
#include <typeinfo>
#include <cstdlib>
#include <cstring>
#include "Config.h"
#include "Format.h"
#include "client/Client.h"
#include "APIRequest.h"
#include "client/HTTP.h"
#include "APIResultParser.h"

APIRequest::APIRequest(std::string url, APIResultParser * parser, ListenerHandle listener, int identifier):
	RequestBroker::Request(API, listener, identifier)
{
	Post = false;
	HTTPContext = NULL;
	Parser = parser;
	URL = url;
}

APIRequest::APIRequest(std::string url, std::map<std::string, std::string> postData, APIResultParser * parser, ListenerHandle listener, int identifier):
	RequestBroker::Request(API, listener, identifier)
{
	Post = true;
	PostData = postData;
	HTTPContext = NULL;
	Parser = parser;
	URL = url;
}

RequestBroker::ProcessResponse APIRequest::Process(RequestBroker & rb)
{
	if(HTTPContext)
	{
		if(http_async_req_status(HTTPContext))
		{
			char * data;
			int status, data_size;
			data = http_async_req_stop(HTTPContext, &status, &data_size);

			Client::Ref().ParseServerReturn(data, status, true);
			if (status == 200 && data)
			{
				void * resultObject = Parser->ProcessResponse((unsigned char *)data, data_size);

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

			if(Client::Ref().GetAuthUser().ID)
			{
				User user = Client::Ref().GetAuthUser();
				char userName[12];
				char *userSession = new char[user.SessionID.length() + 1];
				std::strcpy(userName, format::NumberToString<int>(user.ID).c_str());
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
			if(Client::Ref().GetAuthUser().ID)
			{
				User user = Client::Ref().GetAuthUser();
				char userName[12];
				char *userSession = new char[user.SessionID.length() + 1];
				std::strcpy(userName, format::NumberToString<int>(user.ID).c_str());
				std::strcpy(userSession, user.SessionID.c_str());
				http_auth_headers(HTTPContext, userName, NULL, userSession);
				delete[] userSession;
			}
		}
		//RequestTime = time(NULL);
	}
	return RequestBroker::OK;
}

APIRequest::~APIRequest()
{
	delete Parser;
}

void APIRequest::Cleanup()
{
	Request::Cleanup();
	if(ResultObject)
	{
		Parser->Cleanup(ResultObject);
		ResultObject = NULL;
	}
}
