#include "RemoveTagRequest.h"
#include "Config.h"

namespace http
{
	RemoveTagRequest::RemoveTagRequest(int saveID, ByteString tag) :
		APIRequest({ ByteString::Build(SERVER, "/Browse/EditTag.json"), {
			{ "Op", "delete" },
			{ "ID", ByteString::Build(saveID) },
			{ "Tag", tag },
		} }, authRequireAppendSession, true)
	{
	}

	std::list<ByteString> RemoveTagRequest::Finish()
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
