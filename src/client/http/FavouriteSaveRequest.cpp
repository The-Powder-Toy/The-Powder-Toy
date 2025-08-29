#include "FavouriteSaveRequest.h"
#include "Config.h"

namespace http
{
	static ByteString Url(int saveID, bool favourite)
	{
		ByteStringBuilder builder;
		builder << SERVER << "/Browse/Favourite.json?ID=" << saveID;
		if (!favourite)
		{
			builder << "&Mode=Remove";
		}
		return builder.Build();
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
