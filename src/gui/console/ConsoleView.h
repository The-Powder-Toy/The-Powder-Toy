#pragma once
#include "gui/interface/Window.h"
#include <vector>

namespace ui
{
	class Label;
	class Textbox;
}

class ConsoleController;
class ConsoleModel;
class ConsoleView: public ui::Window
{
	ConsoleController * c;
	ui::Textbox * commandField;
	std::vector<ui::Label*> commandList;
	bool doClose = false;
	String newCommand;
	bool editingNewCommand = true;
public:
	ConsoleView();
	void OnDraw() override;
	void OnTick() override;
	void DoKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void DoTextInput(String text) override;
	void AttachController(ConsoleController * c_) { c = c_; }
	void NotifyPreviousCommandsChanged(ConsoleModel * sender);
	void NotifyCurrentCommandChanged(ConsoleModel * sender);
	virtual ~ConsoleView();
};
