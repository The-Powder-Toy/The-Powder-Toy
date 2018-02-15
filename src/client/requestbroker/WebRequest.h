#include <map>
#include "RequestBroker.h"

class WebRequest: public RequestBroker::Request
{
public:
	bool Post;
	std::string URL;
	std::map<std::string, std::string> PostData;
	void * HTTPContext;
	WebRequest(std::string url, ListenerHandle listener = ListenerHandle(0, (RequestListener*)nullptr), int identifier = 0);
	WebRequest(std::string url, std::map<std::string, std::string>, ListenerHandle listener = ListenerHandle(0, (RequestListener*)nullptr), int identifier = 0);
	RequestBroker::ProcessResponse Process(RequestBroker & rb) override;
	~WebRequest() override;
	void Cleanup() override;
};
