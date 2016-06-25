#include "Favorite.h"
#include <algorithm>

std::vector<std::string> *favoritesList;

Favorite::Favorite()
{
	favoritesList = new std::vector<std::string>();
}


std::vector<std::string> *Favorite::GetFavoritesList()
{
	return favoritesList;
}

void Favorite::SetFavoritesList(std::vector<std::string> newFavoritesList)
{
	*favoritesList = newFavoritesList;
}

bool Favorite::IsFavorite(std::string identifier)
{
	std::vector<std::string> tempFavoritsList = *favoritesList;
	return std::find(tempFavoritsList.begin(), tempFavoritsList.end(), identifier) != tempFavoritsList.end();
}
