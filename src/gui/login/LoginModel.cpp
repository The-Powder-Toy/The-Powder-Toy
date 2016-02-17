#include "LoginModel.h"
#include "Format.h"
#include "Lang.h"

LoginModel::LoginModel():
	currentUser(0, "")
{

}

void LoginModel::Login(string username, string password)
{
	statusText = TEXT_GUI_LOGIN_STATUS_LOGGING;
	loginStatus = false;
	notifyStatusChanged();
	LoginStatus status = Client::Ref().Login(username, password, currentUser);
	switch(status)
	{
	case LoginOkay:
		statusText = TEXT_GUI_LOGIN_STATUS_LOGGED;
		loginStatus = true;
		break;
	case LoginError:
		statusText = format::StringToWString(Client::Ref().GetLastError()); //TODO: Chinese?
		size_t banStart = statusText.find(L". Ban expire in"); //TODO: temporary, remove this when the ban message is fixed
		if (banStart != statusText.npos)
			statusText.replace(banStart, 15, TEXT_GUI_LOGIN_STATUS_BAN);
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
	return format::WStringToString(statusText);
}

wstring LoginModel::GetWStatusText()
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

