#ifndef CHECKBOX_H_
#define CHECKBOX_H_

#include "common/String.h"
#include "Component.h"

#include <functional>

namespace ui
{
class Checkbox: public ui::Component {
	String text;
	String toolTip;
	bool checked;
	bool isMouseOver;
	struct CheckboxAction
	{
		std::function<void ()> action;
	};
	CheckboxAction actionCallback;

public:
	Checkbox(ui::Point position, ui::Point size, String text, String toolTip);
	virtual ~Checkbox() = default;
	
	void SetText(String text);
	String GetText();
	void SetIcon(Icon icon);
	void Draw(const Point& screenPos) override;
	void OnMouseEnter(int x, int y) override;
	void OnMouseHover(int x, int y) override;
	void OnMouseLeave(int x, int y) override;
	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUp(int x, int y, unsigned int button) override;
	inline void SetActionCallback(CheckboxAction const &action) { actionCallback = action; }
	inline CheckboxAction const &GetActionCallback() const { return actionCallback; }
	bool GetChecked() { return checked; }
	void SetChecked(bool checked_) { checked = checked_; }
};
}

#endif /* CHECKBOX_H_ */
