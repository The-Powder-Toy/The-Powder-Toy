/*
 * ElementSearchActivity.h
 *
 *  Created on: Jun 24, 2012
 *      Author: Simon
 */

#ifndef ELEMENTSEARCHACTIVITY_H_
#define ELEMENTSEARCHACTIVITY_H_

#include <vector>
#include <string>
#include "interface/Window.h"
#include "interface/Textbox.h"
#include "game/ToolButton.h"

class Tool;

class GameModel;

class ElementSearchActivity: public ui::Window {
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
	void Exit();
	void SetActiveTool(int selectionState, Tool * tool);
	virtual ~ElementSearchActivity();
	virtual void OnDraw();
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
};

#endif /* ELEMENTSEARCHACTIVITY_H_ */
