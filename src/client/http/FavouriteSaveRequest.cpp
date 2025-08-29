#include "FavouriteSaveRequest.h"
#include "Config.h"

namespace http
{
	static format::Url Url(int saveID, bool favourite)
	{
		format::Url url{ ByteString::Build(SERVER, "/Browse/Favourite.json") };
		url.params["ID"] = ByteString::Build(saveID);
		if (!favourite)
		{
			url.params["Mode"] = "Remove";
		}
		return url;
	}

	FavouriteSaveRequest::FavouriteSaveRequest(int saveID, bool newFavourite) :
		APIRequest(Url(saveID, newFavourite), authRequireAppendSession, true),
		favourite(newFavourite)
	{
	}

	void FavouriteSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}
