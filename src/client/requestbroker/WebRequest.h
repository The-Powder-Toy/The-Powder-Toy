#include <map>
#include "RequestBroker.h"

class WebRequest: public RequestBroker::Request
{
public:
	bool Post;
	std::string URL;
	std::map<std::string, std::string> PostData; 
	void * HTTPContext;
	WebRequest(std::string url, ListenerHandle listener = ListenerHandle(0, (RequestListener*)0), int identifier = 0);
	WebRequest(std::string url, std::map<std::string, std::string>, ListenerHandle listener = ListenerHandle(0, (RequestListener*)0), int identifier = 0);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~WebRequest();
	virtual void Cleanup();
};

