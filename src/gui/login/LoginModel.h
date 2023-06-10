#pragma once
#include "common/String.h"
#include "client/User.h"
#include <vector>
#include <memory>

namespace http
{
	class LoginRequest;
	class LogoutRequest;
}

enum LoginStatus
{
	loginIdle,
	loginWorking,
	loginSucceeded,
};

class LoginView;
class LoginModel
{
	std::unique_ptr<http::LoginRequest> loginRequest;
	std::unique_ptr<http::LogoutRequest> logoutRequest;
	std::vector<LoginView*> observers;
	String statusText;
	LoginStatus loginStatus = loginIdle;
	void notifyStatusChanged();

public:
	void Login(ByteString username, ByteString password);
	void Logout();
	void AddObserver(LoginView * observer);
	String GetStatusText();
	LoginStatus GetStatus() const
	{
		return loginStatus;
	}
	void Tick();
	User GetUser();
	~LoginModel();
};
