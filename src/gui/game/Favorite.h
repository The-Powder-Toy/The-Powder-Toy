#pragma once
#include "Config.h"

#include "common/String.h"
#include <vector>

#include "common/ExplicitSingleton.h"

class Favorite : public ExplicitSingleton<Favorite>
{
	std::vector<ByteString> favoritesList;
public:
	Favorite();

	std::vector<ByteString> GetFavoritesList();
	bool IsFavorite(ByteString identifier);
	bool AnyFavorites();

	void AddFavorite(ByteString identifier);
	void RemoveFavorite(ByteString identifier);

	void SaveFavoritesToPrefs();
	void LoadFavoritesFromPrefs();
};
