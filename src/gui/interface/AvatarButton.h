#ifndef AVATARBUTTON_H_
#define AVATARBUTTON_H_

#include "common/String.h"

#include "Component.h"
#include "graphics/Graphics.h"
#include "gui/interface/Colour.h"
#include "client/http/AvatarRequest.h"
#include "client/http/RequestMonitor.h"

#include <memory>

namespace ui
{
class AvatarButton;
class AvatarButtonAction
{
public:
	virtual void ActionCallback(ui::AvatarButton * sender) {}
	virtual ~AvatarButtonAction() {}
};

class AvatarButton : public Component, public http::RequestMonitor<http::AvatarRequest>
{
	std::unique_ptr<VideoBuffer> avatar;
	ByteString name;
	bool tried;
public:
	AvatarButton(Point position, Point size, ByteString username);
	virtual ~AvatarButton();

	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUnclick(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseLeave(int x, int y) override;

	void OnContextMenuAction(int item) override;

	void Draw(const Point& screenPos) override;
	void Tick(float dt) override;

	void OnResponse(std::unique_ptr<VideoBuffer> avatar) override;

	void DoAction();

	void SetUsername(ByteString username) { name = username; }
	ByteString GetUsername() { return name; }
	void SetActionCallback(AvatarButtonAction * action);
protected:
	bool isMouseInside, isButtonDown;
	AvatarButtonAction * actionCallback;
};
}
#endif /* AVATARBUTTON_H_ */

