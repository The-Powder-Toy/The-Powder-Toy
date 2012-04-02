/*
 * StampsModelException.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef STAMPSMODELEXCEPTION_H_
#define STAMPSMODELEXCEPTION_H_

#include <string>
#include <exception>
using namespace std;

class StampsModelException {
	string message;
public:
	StampsModelException(string message_): message(message_) {};
	const char * what() const throw() { return message.c_str(); };
	~StampsModelException() throw() {};
};

#endif /* STAMPSMODELEXCEPTION_H_ */
