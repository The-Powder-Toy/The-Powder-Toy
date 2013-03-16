#include "RequestBroker.h"

class APIResultParser;
class APIRequest: public RequestBroker::Request
{
public:
	APIResultParser * Parser;
	std::string URL;
	void * HTTPContext;
	APIRequest(std::string url, APIResultParser * parser, ListenerHandle listener = ListenerHandle(0, (RequestListener*)NULL));
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~APIRequest();
	virtual void Cleanup();
};

