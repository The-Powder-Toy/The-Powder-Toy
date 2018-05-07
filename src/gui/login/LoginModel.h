#ifndef LOGINMODEL_H_
#define LOGINMODEL_H_

#include <vector>
#include "common/String.h"
#include "LoginView.h"
#include "client/Client.h"

using namespace std;

class LoginView;
class LoginModel {
	vector<LoginView*> observers;
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

#endif /* LOGINMODEL_H_ */
