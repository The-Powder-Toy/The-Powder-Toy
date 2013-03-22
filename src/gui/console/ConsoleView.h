#ifndef CONSOLEVIEW_H_
#define CONSOLEVIEW_H_

#include <vector>
#include <queue>
#include "gui/interface/Label.h"
#include "gui/interface/Window.h"
#include "ConsoleController.h"
#include "ConsoleModel.h"
#include "gui/interface/Textbox.h"
#include "ConsoleCommand.h"


class ConsoleController;
class ConsoleModel;
class ConsoleView: public ui::Window {
	ConsoleController * c;
	ui::Textbox * commandField;
	std::vector<ui::Label*> commandList;
public:
	ConsoleView();
	virtual void OnDraw();
	virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	void AttachController(ConsoleController * c_) { c = c_; }
	void NotifyPreviousCommandsChanged(ConsoleModel * sender);
	void NotifyCurrentCommandChanged(ConsoleModel * sender);
	virtual ~ConsoleView();
};

#endif /* CONSOLEVIEW_H_ */
