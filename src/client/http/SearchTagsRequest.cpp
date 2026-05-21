#include "SearchTagsRequest.h"
#include "Config.h"
#include "Format.h"

namespace http
{
	static format::Url Url(int start, int count, ByteString query)
	{
		format::Url url{ ByteString::Build(SERVER, "/Browse/Tags.json") };
		url.params["Start"] = ByteString::Build(start);
		url.params["Count"] = ByteString::Build(count);
		if (query.size())
		{
			url.params["Search_Query"] = query;
		}
		return url;
	}

	SearchTagsRequest::SearchTagsRequest(int start, int count, ByteString query) : APIRequest(Url(start, count, query), authOmit, false)
	{
	}

	std::vector<std::pair<ByteString, int>> SearchTagsRequest::Finish()
	{
		std::vector<std::pair<ByteString, int>> tags;
		auto result = APIRequest::Finish();
		try
		{
			for (auto &tag : result["Tags"])
			{
				tags.push_back({
					tag["Tag"].asString(),
					tag["Count"].asInt(),
				});
			}
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return tags;
	}
}
