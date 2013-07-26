#ifndef COPYTEXTBUTTON_H
#define COPYTEXTBUTTON_H

#include "Button.h"
#include "Label.h"

namespace ui
{
class CopyTextButton : public Button
{
	ui::Label *copyTextLabel;
public:
	CopyTextButton(Point position, Point size, std::string buttonText, Label *copyTextLabel_);

	virtual void OnMouseClick(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);
};
}
#endif /* COPYTEXTBUTTON_H */

