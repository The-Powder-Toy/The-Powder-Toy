#include "SearchSavesRequest.h"
#include "Config.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "Format.h"

namespace http
{
	static ByteString Url(int start, int count, ByteString query, Sort sort, Category category)
	{
		ByteStringBuilder builder;
		builder << SCHEME << SERVER << "/Browse.json?Start=" << start << "&Count=" << count;
		auto appendToQuery = [&query](ByteString str) {
			if (query.size())
			{
				query += " ";
			}
			query += str;
		};
		switch (sort)
		{
		case sortByDate:
			appendToQuery("sort:date");
			break;

		default:
			break;
		}
		auto user = Client::Ref().GetAuthUser();
		switch (category)
		{
		case categoryFavourites:
			builder << "&Category=Favourites";
			break;

		case categoryMyOwn:
			assert(user.UserID);
			appendToQuery("user:" + user.Username);
			break;

		default:
			break;
		}
		if (query.size())
		{
			builder << "&Search_Query=" << format::URLEncode(query);
		}
		return builder.Build();
	}

	SearchSavesRequest::SearchSavesRequest(int start, int count, ByteString query, Sort sort, Category category) : APIRequest(Url(start, count, query, sort, category), authUse, false)
	{
	}

	std::pair<int, std::vector<std::unique_ptr<SaveInfo>>> SearchSavesRequest::Finish()
	{
		std::vector<std::unique_ptr<SaveInfo>> saves;
		auto result = APIRequest::Finish();
		int count;
		try
		{
			count = result["Count"].asInt();
			for (auto &save : result["Saves"])
			{
				auto saveInfo = std::make_unique<SaveInfo>(
					save["ID"].asInt(),
					save["Created"].asInt64(),
					save["Updated"].asInt64(),
					save["ScoreUp"].asInt(),
					save["ScoreDown"].asInt(),
					save["Username"].asString(),
					ByteString(save["Name"].asString()).FromUtf8()
				);
				saveInfo->Version = save["Version"].asInt();
				saveInfo->SetPublished(save["Published"].asBool());
				saves.push_back(std::move(saveInfo));
			}
		}
		catch (const std::exception &ex)
		{
			throw RequestError("Could not read response: " + ByteString(ex.what()));
		}
		return std::pair{ count, std::move(saves) };
	}
}
