#include <iostream>
#include <typeinfo>
#include <cstdlib>
#include "APIRequest.h"
#include "client/HTTP.h"
#include "APIResultParser.h"

APIRequest::APIRequest(std::string url, APIResultParser * parser, ListenerHandle listener):
	RequestBroker::Request(API, listener)
{
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

			if (status == 200 && data)
			{
				void * resultObject = Parser->ProcessResponse((unsigned char *)data, data_size);
				free(data);

				if(resultObject)
				{
					this->ResultObject = resultObject;
					rb.requestComplete(this);
					return RequestBroker::Finished;
				}
				else
				{
					std::cout << typeid(*this).name() << " Request for " << URL << " could not be parsed" << status << std::endl;
					return RequestBroker::Failed;
				}
			}
			else
			{
//#ifdef DEBUG
				std::cout << typeid(*this).name() << " Request for " << URL << " failed with status " << status << std::endl;
//#endif	
				if(data)
					free(data);

				return RequestBroker::Failed;
			}
		}
	}
	else 
	{
		std::cout << typeid(*this).name() << " New Request for " << URL << std::endl;
		HTTPContext = http_async_req_start(NULL, (char *)URL.c_str(), NULL, 0, 0);
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