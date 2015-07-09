#ifndef PROFILEACTIVITY_H_
#define PROFILEACTIVITY_H_

#include <string>
#include "Activity.h"
#include "client/requestbroker/RequestListener.h"
#include "client/UserInfo.h"
#include "gui/interface/Window.h"
#include "gui/interface/Label.h"

class ProfileActivity: public WindowActivity, public RequestListener {
	ui::Label * location;
	ui::Label * bio;
	UserInfo info;
	bool editable;
	bool loading;
	bool saving;
	void setUserInfo(UserInfo newInfo);
public:
	ProfileActivity(std::string username);
	virtual ~ProfileActivity();
	virtual void OnResponseReady(void * userDataPtr, int identifier);
	virtual void OnDraw();
	virtual void OnTryExit(ExitMethod method);
};

#endif /* PROFILEACTIVITY_H_ */
