#include <map>
#include "RequestBroker.h"

class APIResultParser;
class APIRequest: public RequestBroker::Request
{
public:
	bool Post;
	APIResultParser * Parser;
	std::string URL;
	std::map<std::string, std::string> PostData;
	void * HTTPContext;
	APIRequest(std::string url, APIResultParser * parser, ListenerHandle listener = ListenerHandle(0, (RequestListener*)nullptr), int identifier = 0);
	APIRequest(std::string url, std::map<std::string, std::string>, APIResultParser * parser, ListenerHandle listener = ListenerHandle(0, (RequestListener*)nullptr), int identifier = 0);
	RequestBroker::ProcessResponse Process(RequestBroker & rb) override;
	~APIRequest() override;
	void Cleanup() override;
};
