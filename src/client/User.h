/*
 * User.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Simon
 */

#ifndef USER_H_
#define USER_H_

#include <string>

class User
{
public:
	int ID;
	std::string Username;
	std::string SessionID;
	std::string SessionKey;
	User(int id, std::string username):
		ID(id),
		Username(username)
	{

	}
};


#endif /* USER_H_ */
