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
		userAgent = ByteString::Build(
			"PowderToy/", SAVE_VERSION, ".", MINOR_VERSION,
			" (", IDENT_PLATFORM,
			"; NO", // Unused, used to be SSE level.
			"; M", MOD_ID,
			"; ", IDENT,
			") TPTPP/", SAVE_VERSION, ".", MINOR_VERSION, ".", BUILD_NUM, IDENT_RELTYPE, ".", SNAPSHOT_ID
		);
	}

	void RequestManager::RegisterRequest(Request &request)
	{
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
