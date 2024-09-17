#include "LoginModel.h"
#include "LoginView.h"
#include "Config.h"
#include "client/Client.h"
#include "client/http/LoginRequest.h"
#include "client/http/LogoutRequest.h"

void LoginModel::Login(const ByteString& username, const ByteString& password)
{
	if (username.Contains("@"))
	{
		statusText = String::Build("Use your Powder Toy account to log in, not your email. If you don't have a Powder Toy account, you can create one at ", SERVER, "/Register.html");
		loginStatus = loginIdle;
		notifyStatusChanged();
		return;
	}
	statusText = "Logging in...";
	loginStatus = loginWorking;
	notifyStatusChanged();
	loginRequest = std::make_unique<http::LoginRequest>(username, password);
	loginRequest->Start();
}

void LoginModel::Logout()
{
	statusText = "Logging out...";
	loginStatus = loginWorking;
	notifyStatusChanged();
	logoutRequest = std::make_unique<http::LogoutRequest>();
	logoutRequest->Start();
}

void LoginModel::AddObserver(LoginView * observer)
{
	observers.push_back(observer);
	notifyStatusChanged();
}

String LoginModel::GetStatusText()
{
	return statusText;
}

void LoginModel::Tick()
{
	if (loginRequest && loginRequest->CheckDone())
	{
		try
		{
			auto info = loginRequest->Finish();
			auto &client = Client::Ref();
			client.SetAuthUser(info.user);
			for (auto &item : info.notifications)
			{
				client.AddServerNotification(item);
			}
			statusText = "Logged in";
			loginStatus = loginSucceeded;
		}
		catch (const http::RequestError &ex)
		{
			statusText = ByteString(ex.what()).FromUtf8();
			loginStatus = loginIdle;
		}
		notifyStatusChanged();
		loginRequest.reset();
	}
	if (logoutRequest && logoutRequest->CheckDone())
	{
		try
		{
			logoutRequest->Finish();
			auto &client = Client::Ref();
			client.SetAuthUser(User(0, ""));
			statusText = "Logged out";
		}
		catch (const http::RequestError &ex)
		{
			statusText = ByteString(ex.what()).FromUtf8();
		}
		loginStatus = loginIdle;
		notifyStatusChanged();
		logoutRequest.reset();
	}
}

void LoginModel::notifyStatusChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyStatusChanged(this);
	}
}

LoginModel::~LoginModel()
{
	// Satisfy std::unique_ptr
}
