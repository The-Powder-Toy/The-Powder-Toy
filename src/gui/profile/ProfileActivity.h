#ifndef PROFILEACTIVITY_H_
#define PROFILEACTIVITY_H_

#include <string>
#include "Activity.h"
#include "client/requestbroker/RequestListener.h"
#include "client/UserInfo.h"
#include "gui/interface/Window.h"

namespace ui
{
class Label;
class ScrollPanel;
}
class ProfileActivity: public WindowActivity, public RequestListener {
	ui::ScrollPanel *scrollPanel;
	ui::Label *location;
	ui::Label *bio;
	UserInfo info;
	bool editable;
	bool loading;
	bool saving;
	bool doError;
	std::string doErrorMessage;
	void setUserInfo(UserInfo newInfo);
public:
	ProfileActivity(std::string username);
	~ProfileActivity() override;
	void OnResponseReady(void * userDataPtr, int identifier) override;
	void OnResponseFailed(int identifier) override;
	void OnTick(float dt) override;
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;

	void ResizeArea();
};

#endif /* PROFILEACTIVITY_H_ */
