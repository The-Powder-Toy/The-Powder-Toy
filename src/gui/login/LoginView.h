#ifndef LOGINVIEW_H_
#define LOGINVIEW_H_

#include "gui/interface/Window.h"
#include "LoginController.h"
#include "LoginModel.h"

namespace ui
{
	class Textbox;
	class Button;
	class Label;
}

class LoginController;
class LoginMode;
class LoginView: public ui::Window {
	LoginController * c;
	ui::Button * loginButton;
	ui::Button * cancelButton;
	ui::Label * titleLabel;
	ui::Label * infoLabel;
	ui::Textbox * usernameField;
	ui::Textbox * passwordField;
	ui::Point targetSize;
public:
	class LoginAction;
	class CancelAction;
	LoginView();
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTryExit(ExitMethod method) override;
	void AttachController(LoginController * c_) { c = c_; }
	void NotifyStatusChanged(LoginModel * sender);
	void OnDraw() override;
	void OnTick(float dt) override;
	virtual ~LoginView();
};

#endif /* LOGINVIEW_H_ */
