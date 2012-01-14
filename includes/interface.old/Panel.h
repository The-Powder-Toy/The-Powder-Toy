/*
 * Panel.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#ifndef PANEL_H_
#define PANEL_H_

#include "interface/Component.h"

namespace ui {

class Panel: public ui::Component {
public:
	Panel(int x, int y, int width, int height);
	virtual ~Panel();
};

} /* namespace ui */
#endif /* PANEL_H_ */
