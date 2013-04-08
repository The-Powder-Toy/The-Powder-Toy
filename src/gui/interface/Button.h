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
	virtual void AltActionCallback(ui::Button * sender) {}
	virtual void MouseEnterCallback(ui::Button * sender) {}
	virtual ~ButtonAction() {}
};

class Button : public Component
{
public:
	Button(Point position = Point(0, 0), Point size = Point(0, 0), std::string buttonText = "", std::string toolTip = "");
	virtual ~Button();

	bool Toggleable;
	bool Enabled;

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);
	//virtual void OnMouseUp(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void Draw(const Point& screenPos);

	virtual void TextPosition();
	inline bool GetState() { return state; }
	virtual void DoAction(); //action of button what ever it may be
	virtual void DoAltAction(); //action of button what ever it may be
	void SetTogglable(bool isTogglable);
	bool GetTogglable();
	TPT_NO_INLINE bool GetToggleState();
	TPT_NO_INLINE void SetToggleState(bool state);
	void SetActionCallback(ButtonAction * action);
	ButtonAction * GetActionCallback() { return actionCallback; }
	void SetText(std::string buttonText);
	void SetIcon(Icon icon);
	inline std::string GetText() { return ButtonText; };
protected:

	std::string toolTip;
	std::string buttonDisplayText;
	std::string ButtonText;

	bool isButtonDown, isAltButtonDown, state, isMouseInside, isTogglable, toggle;
	ButtonAction * actionCallback;

};
}
#endif /* BUTTON_H_ */
