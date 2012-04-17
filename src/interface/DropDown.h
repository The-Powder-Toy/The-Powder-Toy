/*
 * DropDown.h
 *
 *  Created on: Apr 16, 2012
 *      Author: Simon
 */

#ifndef DROPDOWN_H_
#define DROPDOWN_H_

#include "Component.h"
#include "Colour.h"

namespace ui {

class DropDown: public ui::Component {
	Colour background, activeBackground;
	Colour border, activeBorder;
	Colour text, activeText;
	bool isMouseInside;
public:
	DropDown(Point position, Point size);
	virtual void Draw(const Point& screenPos);
	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual ~DropDown();
};

} /* namespace ui */
#endif /* DROPDOWN_H_ */
