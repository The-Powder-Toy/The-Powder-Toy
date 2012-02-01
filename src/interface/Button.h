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
#include "Colour.h"

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
	Button(Point position = Point(0, 0), Point size = Point(0, 0), std::string buttonText = "");
	virtual ~Button();

	Icon icon;
	bool Toggleable;
	bool Enabled;

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUp(int x, int y, unsigned int button);
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

	void SetBackgroundColour(Colour background) { this->background = background; }
	void SetActiveBackgroundColour(Colour background) { this->activeBackground = background; }
	void SetBorderColour(Colour border) { this->border = border; }
	void SetActiveBorderColour(Colour border) { this->activeBorder = border; }
	void SetTextColour(Colour text) { this->text = text; }
	void SetActiveTextColour(Colour text) { this->activeText = text; }

	void SetIcon(Icon icon);
protected:
	Colour background, activeBackground;
	Colour border, activeBorder;
	Colour text, activeText;

	std::string buttonDisplayText;
	std::string ButtonText;

	bool isButtonDown, state, isMouseInside, isTogglable, toggle;
	ButtonAction * actionCallback;
	ui::Point textPosition;
	HorizontalAlignment textHAlign;
	VerticalAlignment textVAlign;

};
}
#endif /* BUTTON_H_ */
