#ifndef USER_H_
#define USER_H_

#include "common/String.h"


class User
{
public:
	enum Elevation
	{
		ElevationAdmin, ElevationModerator, ElevationNone
	};
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


#endif /* USER_H_ */
