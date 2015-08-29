#include "LoginModel.h"

LoginModel::LoginModel():
	currentUser(0, "")
{

}

void LoginModel::Login(string username, string password)
{
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
		size_t banStart = statusText.find(". Ban expire in"); //TODO: temporary, remove this when the ban message is fixed
		if (banStart != statusText.npos)
			statusText.replace(banStart, 15, ". Login at http://powdertoy.co.uk in order to see the full ban reason. Ban expires in");
		break;
	}
	notifyStatusChanged();
}

void LoginModel::AddObserver(LoginView * observer)
{
	observers.push_back(observer);
}

string LoginModel::GetStatusText()
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

