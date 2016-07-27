#ifndef FAVORITE_H
#define FAVORITE_H

#include <string>
#include <vector>

#include "common/Singleton.h"

class Favorite : public Singleton<Favorite>
{
	std::vector<std::string> favoritesList;
public:
	Favorite();

	std::vector<std::string> GetFavoritesList();
	bool IsFavorite(std::string identifier);
	bool AnyFavorites();
	
	void AddFavorite(std::string identifier);
	void RemoveFavorite(std::string identifier);
	
	void SaveFavoritesToPrefs();
	void LoadFavoritesFromPrefs();
};
#endif //FAVORITE_H
