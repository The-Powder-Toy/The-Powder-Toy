/*
 * TPTScriptInterface.h
 *
 *  Created on: Feb 5, 2012
 *      Author: Simon
 */

#ifndef TPTSCRIPTINTERFACE_H_
#define TPTSCRIPTINTERFACE_H_

#include "CommandInterface.h"
#include "TPTSTypes.h"

class TPTScriptInterface: public CommandInterface {
protected:
	AnyType eval(std::deque<std::string> * words);
	AnyType tptS_set(std::deque<std::string> * words);
	ValueType testType(std::string word);
public:
	TPTScriptInterface();
	virtual void Tick() {}
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	virtual ~TPTScriptInterface();
};

#endif /* TPTSCRIPTINTERFACE_H_ */
