#include "RequestManager.h"
#include "client/http/Request.h"

namespace http
{
	std::shared_ptr<RequestHandle> RequestHandle::Create()
	{
		return std::make_shared<RequestHandle>(CtorTag{});
	}

	void RequestManager::RegisterRequestImpl(Request &request)
	{
		request.handle->statusCode = 604;
		request.handle->error = "network support not compiled in";
		request.handle->MarkDone();
	}

	void RequestManager::UnregisterRequestImpl(Request &request)
	{
	}

	RequestManagerPtr RequestManager::Create(Config newConfig)
	{
		return RequestManagerPtr(new RequestManager(newConfig));
	}

	void RequestManagerDeleter::operator ()(RequestManager *ptr) const
	{
		delete ptr;
	}
}
