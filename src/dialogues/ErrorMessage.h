/*
 * ErrorMessage.h
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#ifndef ERRORMESSAGE_H_
#define ERRORMESSAGE_H_

#include "interface/Window.h"

class ErrorMessage: public ui::Window {
public:
	ErrorMessage(std::string title, std::string message);
	virtual void OnDraw();
	virtual ~ErrorMessage();
};

#endif /* ERRORMESSAGE_H_ */
