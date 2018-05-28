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
	bool doClose = false;
public:
	ConsoleView();
	void OnDraw() override;
	void OnTick(float dt) override;
	void DoKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void DoTextInput(String text) override;
	void AttachController(ConsoleController * c_) { c = c_; }
	void NotifyPreviousCommandsChanged(ConsoleModel * sender);
	void NotifyCurrentCommandChanged(ConsoleModel * sender);
	virtual ~ConsoleView();
};

#endif /* CONSOLEVIEW_H_ */
