#ifndef USERINFO_H_
#define USERINFO_H_

#include <string>

class UserInfo
{
public:
	int ID;
	int Age;
	std::string Username;
	std::string Biography;
	std::string Location;
	UserInfo(int id, int age, std::string username, std::string biography, std::string location):
		ID(id),
		Age(age),
		Username(username),
		Biography(biography),
		Location(location)
	{ }
	UserInfo() {}
};


#endif /* USER_H_ */
