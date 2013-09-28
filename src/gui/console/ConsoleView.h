#ifndef CONSOLEVIEW_H_
#define CONSOLEVIEW_H_

#include <vector>
#include <queue>
#include "gui/interface/Label.h"
#include "gui/interface/Window.h"
#include "gui/interface/ScrollPanel.h"
#include "ConsoleController.h"
#include "ConsoleModel.h"
#include "gui/interface/Textbox.h"
#include "ConsoleCommand.h"


class ConsoleController;
class ConsoleModel;
class ConsoleView: public ui::Window {
	ConsoleController * c;
	ui::Textbox * commandField;
	ui::Label * promptLabel;
	ui::ScrollPanel * history;
	ui::Label * commandHistory;
	ui::Label * promptHistory;
public:
	ConsoleView();
	virtual void OnDraw();
	virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	void AttachController(ConsoleController * c_) { c = c_; }
	void NotifyHistoryChanged(ConsoleModel * sender, std::string command, std::string prompthistory, std::string history);
	void ResizePrompt();
	virtual ~ConsoleView();
};

#endif /* CONSOLEVIEW_H_ */
