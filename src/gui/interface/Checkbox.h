#ifndef CHECKBOX_H_
#define CHECKBOX_H_

#include "common/String.h"
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
	String text;
	String toolTip;
	bool checked;
	bool isMouseOver;
	CheckboxAction * actionCallback;
public:
	Checkbox(ui::Point position, ui::Point size, String text, String toolTip);
	void SetText(String text);
	String GetText();
	void SetIcon(Icon icon);
	void Draw(const Point& screenPos);
	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseHover(int x, int y);
	virtual void OnMouseLeave(int x, int y);
	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUp(int x, int y, unsigned int button);
	void SetActionCallback(CheckboxAction * action);
	CheckboxAction * GetActionCallback() { return actionCallback; }
	bool GetChecked() { return checked; }
	void SetChecked(bool checked_) { checked = checked_; }
	virtual ~Checkbox();
};
}

#endif /* CHECKBOX_H_ */
