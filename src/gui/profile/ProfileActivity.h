#pragma once
#include "common/String.h"
#include "Activity.h"
#include "client/UserInfo.h"
#include <memory>

namespace http
{
	class SaveUserInfoRequest;
	class GetUserInfoRequest;
}

namespace ui
{
class Label;
class ScrollPanel;
}
class ProfileActivity: public WindowActivity {
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

	std::unique_ptr<http::SaveUserInfoRequest> saveUserInfoRequest;
	std::unique_ptr<http::GetUserInfoRequest> getUserInfoRequest;

public:
	ProfileActivity(ByteString username);
	virtual ~ProfileActivity();
	void OnTick(float dt) override;
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;

	void ResizeArea();
};
