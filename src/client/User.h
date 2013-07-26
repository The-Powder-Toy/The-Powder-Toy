#ifndef USER_H_
#define USER_H_

#include <string>


class User
{
public:
	enum Elevation
	{
		ElevationAdmin, ElevationModerator, ElevationNone
	};
	int ID;
	std::string Username;
	std::string SessionID;
	std::string SessionKey;
	Elevation UserElevation;
	User(int id, std::string username):
		ID(id),
		Username(username),
		SessionID(""),
		SessionKey(""),
		UserElevation(ElevationNone)
	{

	}
};


#endif /* USER_H_ */
