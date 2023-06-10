#include "FavouriteSaveRequest.h"
#include "client/Client.h"
#include "Config.h"

namespace http
{
	static ByteString Url(int saveID, bool favourite)
	{
		ByteStringBuilder builder;
		builder << SCHEME << SERVER << "/Browse/Favourite.json?ID=" << saveID << "&Key=" << Client::Ref().GetAuthUser().SessionKey;
		if (!favourite)
		{
			builder << "&Mode=Remove";
		}
		return builder.Build();
	}

	FavouriteSaveRequest::FavouriteSaveRequest(int saveID, bool newFavourite) :
		APIRequest(Url(saveID, newFavourite), authRequire, true),
		favourite(newFavourite)
	{
	}

	void FavouriteSaveRequest::Finish()
	{
		APIRequest::Finish();
	}
}
