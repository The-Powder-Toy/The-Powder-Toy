#pragma once
#include "common/String.h"


class User
{
public:
	enum Elevation
	{
		ElevationNone,
		ElevationHalfMod,
		ElevationMod,
		ElevationAdmin,
	};
	static Elevation ElevationFromString(ByteString str);
	static ByteString ElevationToString(Elevation elevation);

	int UserID;
	ByteString Username;
	ByteString SessionID;
	ByteString SessionKey;
	Elevation UserElevation;
	User(int id, ByteString username):
		UserID(id),
		Username(username),
		SessionID(""),
		SessionKey(""),
		UserElevation(ElevationNone)
	{

	}
};

