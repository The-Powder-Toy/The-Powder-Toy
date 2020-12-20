#ifndef BUTTON_H_
#define BUTTON_H_

#include "common/String.h"
#include "Component.h"

#include <functional>

namespace ui
{

class Button : public Component
{
	struct ButtonAction
	{
		std::function<void ()> action, altAction, mouseEnter;
	};
	
public:
	Button(Point position = Point(0, 0), Point size = Point(0, 0), String buttonText = String(), String toolTip = String());
	virtual ~Button() = default;

	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUnclick(int x, int y, unsigned int button) override;
	void OnMouseUp(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseHover(int x, int y) override;
	void OnMouseLeave(int x, int y) override;

	void Draw(const Point& screenPos) override;

	void TextPosition(String) override;
	inline bool GetState() { return state; }
	void DoAction(); //action of button what ever it may be
	void DoAltAction(); //action of button what ever it may be
	void SetTogglable(bool isTogglable);
	bool GetTogglable();
	bool GetToggleState();
	void SetToggleState(bool state);
	inline void SetActionCallback(ButtonAction const &action) { actionCallback = action; }
	// inline ButtonAction const &GetActionCallback() const { return actionCallback; }
	void SetText(String buttonText);
	void SetIcon(Icon icon);
	inline String GetText() { return ButtonText; }
	void SetToolTip(String newToolTip) { toolTip = newToolTip; }

protected:
	String ButtonText;
	String toolTip;
	String buttonDisplayText;

	bool isButtonDown, isAltButtonDown, state, isMouseInside, isTogglable, toggle;
	ButtonAction actionCallback;
};
}
#endif /* BUTTON_H_ */
