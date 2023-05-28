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

	RequestManagerPtr RequestManager::Create(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork)
	{
		return RequestManagerPtr(new RequestManager(newProxy, newCafile, newCapath, newDisableNetwork));
	}

	void RequestManagerDeleter::operator ()(RequestManager *ptr) const
	{
		delete ptr;
	}
}
