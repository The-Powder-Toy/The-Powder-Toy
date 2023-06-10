#include "User.h"

static const std::vector<std::pair<User::Elevation, ByteString>> elevationStrings = {
	{ User::ElevationAdmin  , "Admin"   },
	{ User::ElevationMod    , "Mod"     },
	{ User::ElevationHalfMod, "HalfMod" },
	{ User::ElevationNone   , "None"    },
};

User::Elevation User::ElevationFromString(ByteString str)
{
	auto it = std::find_if(elevationStrings.begin(), elevationStrings.end(), [&str](auto &item) {
		return item.second == str;
	});
	if (it != elevationStrings.end())
	{
		return it->first;
	}
	return ElevationNone;
}

ByteString User::ElevationToString(Elevation elevation)
{
	auto it = std::find_if(elevationStrings.begin(), elevationStrings.end(), [elevation](auto &item) {
		return item.first == elevation;
	});
	if (it != elevationStrings.end())
	{
		return it->second;
	}
	return "None";
}
