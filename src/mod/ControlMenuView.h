#ifndef CONTROLMENUVIEW_H_
#define CONTROLMENUVIEW_H_

#include "gui/interface/Window.h"

namespace ui 
{
	class Button;
	class Label;
}

class ControlMenuController;
class ControlMenuModel;

class ControlMenuView : public ui::Window
{
	ControlMenuController* c;
	ui::Button* loginButton;
	ui::Button* cancelButton;
	ui::Label* titleLabel;
	ui::Label* infoLabel;
	ui::Point targetSize;
public:
	ControlMenuView();
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTryExit(ExitMethod method) override;
	void AttachController(ControlMenuController* c_) { c = c_; }
	void NotifyStatusChanged(ControlMenuModel* sender);
	void OnDraw() override;
	void OnTick(float dt) override;
	virtual ~ControlMenuView() {} ;

};

#endif // CONTROLMENUVIEW_H_

