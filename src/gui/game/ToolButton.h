#ifndef TOOLBUTTON_H_
#define TOOLBUTTON_H_

#include "gui/interface/Button.h"

class ToolButton: public ui::Button
{
	int currentSelection;
	bool unlocked = true;
	std::string toolIdentifier;
public:
	ToolButton(ui::Point position, ui::Point size, std::string text_, std::string toolIdentifier, std::string toolTip = "");
	virtual void OnMouseUnclick(int x, int y, unsigned int button);
	virtual void OnMouseUp(int x, int y, unsigned int button);
	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void Draw(const ui::Point& screenPos);
	void SetSelectionState(int state);
	int GetSelectionState();
	void SetUnlocked(bool unlocked) { this->unlocked = unlocked; }
	bool GetUnlocked() { return unlocked; }
	virtual ~ToolButton();
};

#endif /* TOOLBUTTON_H_ */
