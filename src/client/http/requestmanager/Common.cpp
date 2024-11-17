#include "RequestManager.h"
#include "client/http/Request.h"
#include "Config.h"

namespace http
{
	RequestManager::RequestManager(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork) :
		proxy(newProxy),
		cafile(newCafile),
		capath(newCapath),
		disableNetwork(newDisableNetwork)
	{
		auto apiVersion = Version(97, 0);
		userAgent = ByteString::Build(
			"PowderToy/", DISPLAY_VERSION[0], ".", DISPLAY_VERSION[1],
			" (", IDENT_PLATFORM,
			"; NO", // Unused, used to be SSE level.
			"; M", MOD_ID,
			"; P", PACKAGE_MODE,
			"; ", IDENT,
			") TPTPP/", apiVersion[0], ".", apiVersion[1], ".", APP_VERSION.build, IDENT_RELTYPE, ".", APP_VERSION.build
		);
	}

	void RequestManager::RegisterRequest(Request &request)
	{
		if (request.handle->failEarly)
		{
			request.handle->error = request.handle->failEarly.value();
			request.handle->statusCode = 600;
			request.handle->MarkDone();
			return;
		}
		if (disableNetwork)
		{
			request.handle->statusCode = 604;
			request.handle->error = "network disabled upon request";
			request.handle->MarkDone();
			return;
		}
		RegisterRequestImpl(request);
	}

	void RequestManager::UnregisterRequest(Request &request)
	{
		UnregisterRequestImpl(request);
	}
}
