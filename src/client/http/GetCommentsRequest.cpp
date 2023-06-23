#include "GetCommentsRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	GetCommentsRequest::GetCommentsRequest(int saveID, int start, int count) :
		APIRequest(ByteString::Build(SCHEME, SERVER, "/Browse/Comments.json?ID=", saveID, "&Start=", start, "&Count=", count), authOmit, false)
	{
	}

	std::vector<Comment> GetCommentsRequest::Finish()
	{
		auto result = APIRequest::Finish();
		std::vector<Comment> comments;
		auto user = Client::Ref().GetAuthUser();
		try
		{
			for (auto &comment : result)
			{
				comments.push_back({
					comment["Username"].asString(),
					User::ElevationFromString(comment["Elevation"].asString()),
					comment["Username"].asString() == user.Username,
					comment["IsBanned"].asBool(),
					ByteString(comment["Text"].asString()).FromUtf8(),
				});
			}
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return comments;
	}
}
