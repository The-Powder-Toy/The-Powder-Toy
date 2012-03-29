/*
 * TagsModelException.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef TAGSMODELEXCEPTION_H_
#define TAGSMODELEXCEPTION_H_

#include <string>
#include <exception>
using namespace std;

class TagsModelException {
	string message;
public:
	TagsModelException(string message_): message(message_) {};
	const char * what() const throw() { return message.c_str(); };
	~TagsModelException() throw() {};
};

#endif /* TAGSMODELEXCEPTION_H_ */
