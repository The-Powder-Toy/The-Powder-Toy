/*
 * LoginModel.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#include "LoginModel.h"

LoginModel::LoginModel():
	currentUser(0, "")
{
	// TODO Auto-generated constructor stub

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
		statusText = "Error: " + Client::Ref().GetLastError();
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
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyStatusChanged(this);
	}
}

LoginModel::~LoginModel() {
	// TODO Auto-generated destructor stub
}

