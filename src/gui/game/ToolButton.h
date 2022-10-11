#ifndef TOOLBUTTON_H_
#define TOOLBUTTON_H_

#include "gui/interface/Button.h"

class Tool;

class ToolButton: public ui::Button
{
	int currentSelection;
	ByteString toolIdentifier;
public:
	ToolButton(ui::Point position, ui::Point size, String text, ByteString toolIdentifier, String toolTip = String());
	void OnMouseUnclick(int x, int y, unsigned int button) override;
	void OnMouseUp(int x, int y, unsigned int button) override;
	void OnMouseClick(int x, int y, unsigned int button) override;
	void Draw(const ui::Point& screenPos) override;
	void SetSelectionState(int state);
	int GetSelectionState();
	Tool *tool;
	int clipRectX = 0;
	int clipRectY = 0;
	int clipRectW = 0;
	int clipRectH = 0;
};

#endif /* TOOLBUTTON_H_ */
