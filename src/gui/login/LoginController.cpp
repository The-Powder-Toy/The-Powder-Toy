#include "LoginController.h"
#include "client/User.h"
#include "client/Client.h"

LoginController::LoginController(ControllerCallback * callback):
	HasExited(false)
{
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
	}
	Client::Ref().SetAuthUser(loginModel->GetUser());
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}

LoginController::~LoginController() {
	if(ui::Engine::Ref().GetWindow() == loginView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	delete loginModel;
	delete loginView;
}

