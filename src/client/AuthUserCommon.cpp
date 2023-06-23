#include "Client.h"
#include "prefs/GlobalPrefs.h"

void Client::LoadAuthUser()
{
	auto &prefs = GlobalPrefs::Ref();
	authUser.UserID = prefs.Get("User.ID", 0);
	authUser.Username = prefs.Get("User.Username", ByteString(""));
	authUser.SessionID = prefs.Get("User.SessionID", ByteString(""));
	authUser.SessionKey = prefs.Get("User.SessionKey", ByteString(""));
	authUser.UserElevation = prefs.Get("User.Elevation", User::ElevationNone);
}

void Client::SaveAuthUser()
{
	auto &prefs = GlobalPrefs::Ref();
	Prefs::DeferWrite dw(prefs);
	if (authUser.UserID)
	{
		prefs.Set("User.ID", authUser.UserID);
		prefs.Set("User.SessionID", authUser.SessionID);
		prefs.Set("User.SessionKey", authUser.SessionKey);
		prefs.Set("User.Username", authUser.Username);
		prefs.Set("User.Elevation", authUser.UserElevation);
	}
	else
	{
		prefs.Clear("User");
	}
}
