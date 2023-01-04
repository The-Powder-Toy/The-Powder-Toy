#pragma once
#include "Config.h"

#include <vector>
#include "common/String.h"
#include "client/User.h"

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
