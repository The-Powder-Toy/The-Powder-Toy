#include "Favorite.h"
#include "prefs/GlobalPrefs.h"
#include <json/json.h>
#include <algorithm>

Favorite::Favorite():
    favoritesList(std::vector<ByteString>())
{}


std::vector<ByteString> Favorite::GetFavoritesList()
{
	return favoritesList;
}

bool Favorite::IsFavorite(ByteString identifier)
{
	return std::find(favoritesList.begin(), favoritesList.end(), identifier) != favoritesList.end();
}

bool Favorite::AnyFavorites()
{
	return favoritesList.size() != 0;
}

void Favorite::AddFavorite(ByteString identifier)
{
	if (!IsFavorite(identifier))
	{
		favoritesList.push_back(identifier);
		SaveFavoritesToPrefs();
	}
}

void Favorite::RemoveFavorite(ByteString identifier)
{
	favoritesList.erase(std::remove(favoritesList.begin(), favoritesList.end(), identifier), favoritesList.end());
	SaveFavoritesToPrefs();
}

void Favorite::SaveFavoritesToPrefs()
{
	GlobalPrefs::Ref().Set("Favorites", favoritesList);
}

void Favorite::LoadFavoritesFromPrefs()
{
	favoritesList = GlobalPrefs::Ref().Get("Favorites", std::vector<ByteString>{});
}
