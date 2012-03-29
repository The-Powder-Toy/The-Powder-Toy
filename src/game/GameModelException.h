/*
 * SaveLoadException.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef SAVELOADEXCEPTION_H_
#define SAVELOADEXCEPTION_H_

#include <string>
#include <exception>
using namespace std;

struct GameModelException: public exception {
	string message;
public:
	GameModelException(string message_): message(message_) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~GameModelException() throw() {};
};

#endif /* SAVELOADEXCEPTION_H_ */
