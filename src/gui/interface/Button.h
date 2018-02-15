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
	virtual ~ButtonAction() = default;
};

class Button : public Component
{
public:
	Button(Point position = Point(0, 0), Point size = Point(0, 0), std::string buttonText = "", std::string toolTip = "");
	~Button() override;

	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUnclick(int x, int y, unsigned int button) override;
	void OnMouseUp(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseHover(int x, int y) override;
	void OnMouseLeave(int x, int y) override;

	void Draw(const Point& screenPos) override;

	void TextPosition(std::string) override;
	inline bool GetState() { return state; }
	virtual void DoAction(); //action of button what ever it may be
	virtual void DoAltAction(); //action of button what ever it may be
	void SetTogglable(bool isTogglable);
	bool GetTogglable();
	bool GetToggleState();
	void SetToggleState(bool state);
	void SetActionCallback(ButtonAction * action);
	ButtonAction * GetActionCallback() { return actionCallback; }
	void SetText(std::string buttonText);
	void SetIcon(Icon icon);
	inline std::string GetText() { return ButtonText; }
	void SetToolTip(std::string newToolTip) { toolTip = newToolTip; }
protected:

	std::string ButtonText;
	std::string toolTip;
	std::string buttonDisplayText;

	bool isButtonDown, isAltButtonDown, state, isMouseInside, isTogglable, toggle;
	ButtonAction * actionCallback;

};
}
#endif /* BUTTON_H_ */
