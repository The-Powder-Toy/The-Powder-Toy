#ifndef ELEMENTSEARCHACTIVITY_H_
#define ELEMENTSEARCHACTIVITY_H_

#include <vector>
#include <string>
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
	bool shiftPressed;
	bool ctrlPressed;
	bool altPressed;
	void searchTools(std::string query);

public:
	class ToolAction;
	bool exit;
	Tool * GetFirstResult() { return firstResult; }
	ElementSearchActivity(GameController * gameController, std::vector<Tool*> tools);
	void SetActiveTool(int selectionState, Tool * tool);
	virtual ~ElementSearchActivity();
	virtual void OnDraw();
	virtual void OnTick(float dt);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
};

#endif /* ELEMENTSEARCHACTIVITY_H_ */
