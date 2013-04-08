#ifndef ELEMENTSEARCHACTIVITY_H_
#define ELEMENTSEARCHACTIVITY_H_

#include <vector>
#include <string>
#include "Activity.h"
#include "gui/interface/Window.h"
#include "gui/interface/Textbox.h"
#include "gui/game/ToolButton.h"

class Tool;

class GameModel;

class ElementSearchActivity: public WindowActivity {
	Tool * firstResult;
	GameModel * gameModel;
	std::vector<Tool*> tools;
	ui::Textbox * searchField;
	std::vector<ToolButton*> toolButtons;
	void searchTools(std::string query);
public:
	class ToolAction;
	Tool * GetFirstResult() { return firstResult; }
	ElementSearchActivity(GameModel * gameModel, std::vector<Tool*> tools);
	void SetActiveTool(int selectionState, Tool * tool);
	virtual ~ElementSearchActivity();
	virtual void OnDraw();
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
};

#endif /* ELEMENTSEARCHACTIVITY_H_ */
