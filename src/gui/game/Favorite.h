#ifndef FAVORITE_H
#define FAVORITE_H

#include "common/String.h"
#include <vector>

#include "common/Singleton.h"

class Favorite : public Singleton<Favorite>
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
#endif //FAVORITE_H
