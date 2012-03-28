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

struct SaveLoadException: public exception {
	string message;
public:
	SaveLoadException(string message_): message(message_) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~SaveLoadException() throw() {};
};

#endif /* SAVELOADEXCEPTION_H_ */
