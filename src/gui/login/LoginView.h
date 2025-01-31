#pragma once
#include "gui/interface/Window.h"
#include "gui/interface/Fade.h"

namespace ui
{
	class Textbox;
	class Button;
	class Label;
}

class LoginController;
class LoginModel;
class LoginView: public ui::Window
{
	LoginController *c{};
	ui::Button *loginButton{};
	ui::Button *cancelButton{};
	ui::Label *titleLabel{};
	ui::Label *infoLabel{};
	ui::Textbox *usernameField{};
	ui::Textbox *passwordField{};
	ui::Fade targetSize{ ui::Fade::BasicDimensionProfile };
public:
	LoginView();
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTryExit(ExitMethod method) override;
	void AttachController(LoginController * c_) { c = c_; }
	void NotifyStatusChanged(LoginModel * sender);
	void OnDraw() override;
	void OnTick() override;
};
