/*
 * Window.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include "interface/State.h"

namespace ui {

class Window: public ui::State {
public:
	Window();
	virtual ~Window();
};

} /* namespace ui */
#endif /* WINDOW_H_ */
