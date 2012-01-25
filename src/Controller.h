/*
 * Controller.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Simon
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

class ControllerCallback
{
public:
	ControllerCallback() {}
	virtual void ControllerExit() {}
	virtual ~ControllerCallback() {}
};

#endif /* CONTROLLER_H_ */
