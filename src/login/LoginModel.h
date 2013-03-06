#ifndef LOGINMODEL_H_
#define LOGINMODEL_H_

#include <vector>
#include <string>
#include "LoginView.h"
#include "client/Client.h"

using namespace std;

class LoginView;
class LoginModel {
	vector<LoginView*> observers;
	string statusText;
	bool loginStatus;
	void notifyStatusChanged();
	User currentUser;
public:
	LoginModel();
	void Login(string username, string password);
	void AddObserver(LoginView * observer);
	string GetStatusText();
	bool GetStatus();
	User GetUser();
	virtual ~LoginModel();
};

#endif /* LOGINMODEL_H_ */
