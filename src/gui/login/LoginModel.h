#pragma once
#include "common/String.h"
#include "client/User.h"
#include <vector>

class LoginView;
class LoginModel
{
	std::vector<LoginView*> observers;
	String statusText;
	bool loginStatus;
	void notifyStatusChanged();
	User currentUser;
public:
	LoginModel();
	void Login(ByteString username, ByteString password);
	void AddObserver(LoginView * observer);
	String GetStatusText();
	bool GetStatus();
	User GetUser();
	virtual ~LoginModel();
};
