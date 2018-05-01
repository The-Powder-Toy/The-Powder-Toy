#include "LoginModel.h"

LoginModel::LoginModel():
	currentUser(0, "")
{

}

void LoginModel::Login(ByteString username, ByteString password)
{
	if (username.Contains("@"))
	{
		statusText = "Use your Powder Toy account to log in, not your email. If you don't have a Powder Toy account, you can create one at https://powdertoy.co.uk/Register.html";
		loginStatus = false;
		notifyStatusChanged();
		return;
	}
	statusText = "Logging in...";
	loginStatus = false;
	notifyStatusChanged();
	LoginStatus status = Client::Ref().Login(username, password, currentUser);
	switch(status)
	{
	case LoginOkay:
		statusText = "Logged in";
		loginStatus = true;
		break;
	case LoginError:
		statusText = Client::Ref().GetLastError();
		break;
	}
	notifyStatusChanged();
}

void LoginModel::AddObserver(LoginView * observer)
{
	observers.push_back(observer);
}

String LoginModel::GetStatusText()
{
	return statusText;
}

User LoginModel::GetUser()
{
	return currentUser;
}

bool LoginModel::GetStatus()
{
	return loginStatus;
}

void LoginModel::notifyStatusChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyStatusChanged(this);
	}
}

LoginModel::~LoginModel() {
}

