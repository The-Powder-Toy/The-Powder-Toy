#pragma once
#include "APIRequest.h"

namespace http
{
	class FavouriteSaveRequest : public APIRequest
	{
		bool favourite;

	public:
		FavouriteSaveRequest(int saveID, bool newFavourite);

		void Finish();

		bool Favourite() const
		{
			return favourite;
		}
	};
}
