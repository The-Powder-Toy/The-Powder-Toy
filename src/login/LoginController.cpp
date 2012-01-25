/*
 * LoginController.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#include "LoginController.h"
#include "client/User.h"

LoginController::LoginController(ControllerCallback * callback) {
	// TODO Auto-generated constructor stub
	loginView = new LoginView();
	loginModel = new LoginModel();

	loginView->AttachController(this);
	loginModel->AddObserver(loginView);

	this->callback = callback;

}

void LoginController::Login(string username, string password)
{
	loginModel->Login(username, password);
}

User LoginController::GetUser()
{
	return loginModel->GetUser();
}

void LoginController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == loginView)
	{
		ui::Engine::Ref().CloseWindow();
		loginView = NULL;
	}
	if(callback)
		callback->ControllerExit();
}

LoginController::~LoginController() {
	if(loginView)
		delete loginView;
	delete loginModel;
}

