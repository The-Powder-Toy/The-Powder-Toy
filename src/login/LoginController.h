/*
 * LoginController.h
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#ifndef LOGINCONTROLLER_H_
#define LOGINCONTROLLER_H_

#include <string>
#include "LoginView.h"
#include "LoginModel.h"

using namespace std;

class LoginView;
class LoginModel;
class LoginController {
	LoginView * loginView;
	LoginModel * loginModel;
public:
	LoginController();
	void Login(string username, string password);
	void Exit();
	LoginView * GetView() { return loginView; }

	virtual ~LoginController();
};

#endif /* LOGINCONTROLLER_H_ */
