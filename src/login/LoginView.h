/*
 * LoginView.h
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#ifndef LOGINVIEW_H_
#define LOGINVIEW_H_

#include "interface/Button.h"
#include "interface/Window.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "LoginController.h"
#include "LoginModel.h"

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
public:
	class LoginAction;
	class CancelAction;
	LoginView();
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	void AttachController(LoginController * c_) { c = c_; }
	void NotifyStatusChanged(LoginModel * sender);
	virtual void OnDraw();
	virtual ~LoginView();
};

#endif /* LOGINVIEW_H_ */
