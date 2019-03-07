#ifndef PROFILEACTIVITY_H_
#define PROFILEACTIVITY_H_

#include "common/String.h"
#include "Activity.h"
#include "client/UserInfo.h"
#include "gui/interface/Window.h"
#include "client/SaveUserInfoRequest.h"
#include "client/GetUserInfoRequest.h"
#include "client/RequestMonitor.h"

namespace ui
{
class Label;
class ScrollPanel;
}
using SaveUserInfoRequestMonitor = http::RequestMonitor<http::SaveUserInfoRequest>;
using GetUserInfoRequestMonitor = http::RequestMonitor<http::GetUserInfoRequest>;
class ProfileActivity: public WindowActivity, public SaveUserInfoRequestMonitor, public GetUserInfoRequestMonitor {
	ui::ScrollPanel *scrollPanel;
	ui::Label *location;
	ui::Label *bio;
	UserInfo info;
	bool editable;
	bool loading;
	bool saving;
	bool doError;
	String doErrorMessage;
	void setUserInfo(UserInfo newInfo);
public:
	ProfileActivity(ByteString username);
	virtual ~ProfileActivity();
	virtual void OnTick(float dt);
	virtual void OnDraw();
	virtual void OnTryExit(ExitMethod method);

	void OnResponse(bool saveUserInfoStatus) override;
	void OnResponse(std::unique_ptr<UserInfo> getUserInfoResult) override;

	void ResizeArea();
};

#endif /* PROFILEACTIVITY_H_ */
