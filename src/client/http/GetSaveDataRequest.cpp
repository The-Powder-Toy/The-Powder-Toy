#include "GetSaveDataRequest.h"
#include "Config.h"

namespace http
{
	static ByteString Url(int saveID, int saveDate)
	{
		ByteStringBuilder builder;
		builder << STATICSERVER << "/" << saveID;
		if (saveDate)
		{
			builder << "_" << saveDate;
		}
		builder << ".cps";
		return builder.Build();
	}

	GetSaveDataRequest::GetSaveDataRequest(int saveID, int saveDate) : Request(Url(saveID, saveDate))
	{
	}

	std::vector<char> GetSaveDataRequest::Finish()
	{
		auto [ status, data ] = Request::Finish();
		ParseResponse(data, status, responseData);
		return std::vector<char>(data.begin(), data.end());
	}
}
