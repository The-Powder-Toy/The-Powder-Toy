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
	UserInfo(int id, int age, std::string username, std::string biography):
		ID(id),
		Age(age),
		Username(username),
		Biography(biography)
	{ }
};


#endif /* USER_H_ */
