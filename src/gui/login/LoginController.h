#pragma once
#include "common/String.h"
#include "client/User.h"
#include <functional>

class LoginView;
class LoginModel;
class LoginController
{
	LoginView * loginView;
	LoginModel * loginModel;
	std::function<void ()> onDone;
public:
	bool HasExited;
	LoginController(std::function<void ()> onDone = nullptr);
	void Login(ByteString username, ByteString password);
	void Logout();
	void Tick();
	void Exit();
	LoginView * GetView() { return loginView; }
	~LoginController();
};
