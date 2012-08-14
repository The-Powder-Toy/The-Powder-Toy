/*
 * ToolButton.h
 *
 *  Created on: Jan 30, 2012
 *      Author: Simon
 */

#ifndef TOOLBUTTON_H_
#define TOOLBUTTON_H_

#include "interface/Button.h"

class ToolButton: public ui::Button {
	int currentSelection;
public:
	ToolButton(ui::Point position, ui::Point size, std::string text_, std::string toolTip = "");
	virtual void OnMouseUnclick(int x, int y, unsigned int button);
	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void Draw(const ui::Point& screenPos);
	void SetSelectionState(int state);
	int GetSelectionState();
	virtual ~ToolButton();
};

#endif /* TOOLBUTTON_H_ */
