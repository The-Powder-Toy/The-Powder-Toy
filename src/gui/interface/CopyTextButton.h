#pragma once
#include "Button.h"

namespace ui
{
	class Label;
class CopyTextButton : public Button
{
	ui::Label *copyTextLabel;
public:
	CopyTextButton(Point position, Point size, String buttonText, Label *copyTextLabel_);

	void OnMouseClick(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseLeave(int x, int y) override;
};
}
