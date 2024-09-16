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
	bool IsFavorite(const ByteString& identifier);
	bool AnyFavorites();

	void AddFavorite(const ByteString& identifier);
	void RemoveFavorite(const ByteString& identifier);

	void SaveFavoritesToPrefs();
	void LoadFavoritesFromPrefs();
};
