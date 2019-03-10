#ifndef ELEMENTSEARCHACTIVITY_H_
#define ELEMENTSEARCHACTIVITY_H_

#include <vector>
#include "Activity.h"
#include "gui/interface/Window.h"
#include "gui/interface/Textbox.h"
#include "gui/game/ToolButton.h"

class Tool;
class GameController;

class ElementSearchActivity: public WindowActivity
{
	Tool * firstResult;
	GameController * gameController;
	std::vector<Tool*> tools;
	ui::Textbox * searchField;
	std::vector<ToolButton*> toolButtons;
	String toolTip;
	int toolTipPresence;
	bool shiftPressed;
	bool ctrlPressed;
	bool altPressed;
	bool isToolTipFadingIn;
	void searchTools(String query);

public:
	class ToolAction;
	bool exit;
	Tool * GetFirstResult() { return firstResult; }
	ElementSearchActivity(GameController * gameController, std::vector<Tool*> tools);
	void SetActiveTool(int selectionState, Tool * tool);
	virtual ~ElementSearchActivity();
	void OnTick(float dt) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnDraw() override;
	void ToolTip(ui::Point senderPosition, String ToolTip) override;
};

#endif /* ELEMENTSEARCHACTIVITY_H_ */
