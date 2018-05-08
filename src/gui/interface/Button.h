#ifndef BUTTON_H_
#define BUTTON_H_

#include "common/String.h"
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
	virtual void AltActionCallback(ui::Button * sender) {}
	virtual void MouseEnterCallback(ui::Button * sender) {}
	virtual ~ButtonAction() {}
};

class Button : public Component
{
public:
	Button(Point position = Point(0, 0), Point size = Point(0, 0), String buttonText = String(), String toolTip = String());
	virtual ~Button();

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);
	virtual void OnMouseUp(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseHover(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void Draw(const Point& screenPos);

	virtual void TextPosition(String);
	inline bool GetState() { return state; }
	virtual void DoAction(); //action of button what ever it may be
	virtual void DoAltAction(); //action of button what ever it may be
	void SetTogglable(bool isTogglable);
	bool GetTogglable();
	bool GetToggleState();
	void SetToggleState(bool state);
	void SetActionCallback(ButtonAction * action);
	ButtonAction * GetActionCallback() { return actionCallback; }
	void SetText(String buttonText);
	void SetIcon(Icon icon);
	inline String GetText() { return ButtonText; }
	void SetToolTip(String newToolTip) { toolTip = newToolTip; }
protected:

	String ButtonText;
	String toolTip;
	String buttonDisplayText;

	bool isButtonDown, isAltButtonDown, state, isMouseInside, isTogglable, toggle;
	ButtonAction * actionCallback;

};
}
#endif /* BUTTON_H_ */
