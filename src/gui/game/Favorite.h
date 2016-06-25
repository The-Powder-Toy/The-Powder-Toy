#ifndef FAVORITE_H
#define FAVORITE_H

#include <string>
#include <vector>

#include "common/Singleton.h"

class Favorite : public Singleton<Favorite> {
public:
	Favorite();
	std::vector<std::string> * GetFavoritesList();
	void SetFavoritesList(std::vector<std::string> favoritesList);
	bool IsFavorite(std::string identifier);
};
#endif //FAVORITE_H
