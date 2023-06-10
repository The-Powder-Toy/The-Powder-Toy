#include "AddTagRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	AddTagRequest::AddTagRequest(int saveID, ByteString tag) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Browse/EditTag.json?Op=add&ID=", saveID, "&Tag=", tag, "&Key=", Client::Ref().GetAuthUser().SessionKey), authRequire, true)
	{
	}

	std::list<ByteString> AddTagRequest::Finish()
	{
		auto result = APIRequest::Finish();
		std::list<ByteString> tags;
		try
		{
			for (auto &tag : result["Tags"])
			{
				tags.push_back(tag.asString());
			}
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return tags;
	}
}
