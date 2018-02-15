#ifndef USER_H_
#define USER_H_

#include <string>
#include <utility>


class User
{
public:
	enum Elevation
	{
		ElevationAdmin, ElevationModerator, ElevationNone
	};
	int UserID;
	std::string Username;
	std::string SessionID;
	std::string SessionKey;
	Elevation UserElevation;
	User(int id, std::string username):
		UserID(id),
		Username(std::move(username)),
		SessionID(""),
		SessionKey(""),
		UserElevation(ElevationNone)
	{

	}
};


#endif /* USER_H_ */
