#include "Favorite.h"
#include "json/json.h"
#include "client/Client.h"
#include <algorithm>

Favorite::Favorite():
    favoritesList(std::vector<std::string>())
{}


std::vector<std::string> Favorite::GetFavoritesList()
{
	return favoritesList;
}

bool Favorite::IsFavorite(std::string identifier)
{
	return std::find(favoritesList.begin(), favoritesList.end(), identifier) != favoritesList.end();
}

bool Favorite::AnyFavorites()
{
	return favoritesList.size() == 0;
}

void Favorite::AddFavorite(std::string identifier)
{
	if (!IsFavorite(identifier))
	{
		favoritesList.push_back(identifier);
	}
}

void Favorite::RemoveFavorite(std::string identifier)
{
	favoritesList.erase(std::remove(favoritesList.begin(), favoritesList.end(), identifier), favoritesList.end());
}

void Favorite::SaveFavoritesToPrefs()
{
	Client::Ref().SetPref("Favorites", std::vector<Json::Value>(favoritesList.begin(), favoritesList.end()));
}

void Favorite::LoadFavoritesFromPrefs()
{
	favoritesList = Client::Ref().GetPrefStringArray("Favorites");
}
