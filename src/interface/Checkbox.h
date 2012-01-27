/*
 * Checkbox.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#ifndef CHECKBOX_H_
#define CHECKBOX_H_

#include <string>
#include "Component.h"
namespace ui
{
class Checkbox;
class CheckboxAction
{
public:
	virtual void ActionCallback(ui::Checkbox * sender) {}
	virtual ~CheckboxAction() {}
};
class Checkbox: public ui::Component {
	std::string text;
	bool checked;
	bool isMouseOver;
	CheckboxAction * actionCallback;
public:
	Checkbox(ui::Point position, ui::Point size, std::string text);
	void SetText(std::string text);
	void Draw(const Point& screenPos);
	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);
	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUp(int x, int y, unsigned int button);
	void SetActionCallback(CheckboxAction * action);
	CheckboxAction * GetActionCallback() { return actionCallback; }
	bool IsChecked() { return checked; }
	virtual ~Checkbox();
};
}

#endif /* CHECKBOX_H_ */
