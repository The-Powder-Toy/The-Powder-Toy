/*
 * Button.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <string>
#include "Misc.h"
#include "Component.h"

namespace ui
{
class Button;
class ButtonAction
{
public:
	virtual void ActionCallback(ui::Button * sender) {}
	virtual ~ButtonAction() {}
};

class Button : public Component
{
public:
	Button(Window* parent_state, std::string buttonText);

	Button(Point position, Point size, std::string buttonText);

	Button(std::string buttonText);
	virtual ~Button();

	bool Toggleable;
	bool Enabled;

	std::string ButtonText;

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);
	//virtual void OnMouseUp(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void Draw(const Point& screenPos);

	inline bool GetState() { return state; }
	virtual void DoAction(); //action of button what ever it may be
	void SetTogglable(bool isTogglable);
	bool GetTogglable();
	inline bool GetToggleState();
	inline void SetToggleState(bool state);
	void SetActionCallback(ButtonAction * action);
	ButtonAction * GetActionCallback() { return actionCallback; }
	void TextPosition();
	void SetText(std::string buttonText);
	HorizontalAlignment GetHAlignment() { return textHAlign; }
	VerticalAlignment GetVAlignment() { return textVAlign; }
	void SetAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign) { textHAlign = hAlign; textVAlign = vAlign; TextPosition(); }
protected:
	bool isButtonDown, state, isMouseInside, isTogglable, toggle;
	ButtonAction * actionCallback;
	ui::Point textPosition;
	HorizontalAlignment textHAlign;
	VerticalAlignment textVAlign;

};
}
#endif /* BUTTON_H_ */
