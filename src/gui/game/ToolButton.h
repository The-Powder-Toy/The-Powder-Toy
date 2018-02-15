#ifndef TOOLBUTTON_H_
#define TOOLBUTTON_H_

#include "gui/interface/Button.h"

class ToolButton: public ui::Button
{
	int currentSelection;
	std::string toolIdentifier;
public:
	ToolButton(ui::Point position, ui::Point size, std::string text_, std::string toolIdentifier, std::string toolTip = "");
	void OnMouseUnclick(int x, int y, unsigned int button) override;
	void OnMouseUp(int x, int y, unsigned int button) override;
	void OnMouseClick(int x, int y, unsigned int button) override;
	void Draw(const ui::Point& screenPos) override;
	void SetSelectionState(int state);
	int GetSelectionState();
	~ToolButton() override;
};

#endif /* TOOLBUTTON_H_ */
