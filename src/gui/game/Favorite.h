#pragma once
#include "common/String.h"
#include "common/ExplicitSingleton.h"
#include <vector>

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
