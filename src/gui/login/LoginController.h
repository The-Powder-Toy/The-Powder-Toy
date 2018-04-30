#ifndef LOGINCONTROLLER_H_
#define LOGINCONTROLLER_H_

#include "common/String.h"
#include "LoginView.h"
#include "LoginModel.h"
#include "Controller.h"
#include "client/User.h"

using namespace std;

class LoginView;
class LoginModel;
class LoginController {
	LoginView * loginView;
	LoginModel * loginModel;
	ControllerCallback * callback;
public:
	bool HasExited;
	LoginController(ControllerCallback * callback = NULL);
	void Login(ByteString username, ByteString password);
	void Exit();
	LoginView * GetView() { return loginView; }
	User GetUser();
	virtual ~LoginController();
};

#endif /* LOGINCONTROLLER_H_ */
