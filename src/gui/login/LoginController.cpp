#include "LoginController.h"
#include "client/Client.h"
#include "client/http/LoginRequest.h"
#include "client/http/LogoutRequest.h"
#include "LoginView.h"
#include "LoginModel.h"
#include "Controller.h"

LoginController::LoginController(std::function<void ()> onDone_):
	HasExited(false)
{
	loginView = new LoginView();
	loginModel = new LoginModel();

	loginView->AttachController(this);
	loginModel->AddObserver(loginView);

	onDone = onDone_;
}

void LoginController::Login(ByteString username, ByteString password)
{
	loginModel->Login(username, password);
}

void LoginController::Logout()
{
	loginModel->Logout();
}

void LoginController::Tick()
{
	loginModel->Tick();
}

void LoginController::Exit()
{
	loginView->CloseActiveWindow();
	if (onDone)
		onDone();
	HasExited = true;
}

LoginController::~LoginController()
{
	delete loginModel;
	if (loginView->CloseActiveWindow())
	{
		delete loginView;
	}
}

