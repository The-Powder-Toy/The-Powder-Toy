#ifndef AVATARBUTTON_H_
#define AVATARBUTTON_H_

#include "common/String.h"

#include "Component.h"
#include "graphics/Graphics.h"
#include "gui/interface/Colour.h"
#include "client/requestbroker/RequestListener.h"

namespace ui
{
class AvatarButton;
class AvatarButtonAction
{
public:
	virtual void ActionCallback(ui::AvatarButton * sender) {}
	virtual ~AvatarButtonAction() {}
};

class AvatarButton : public Component, public RequestListener
{
	VideoBuffer * avatar;
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

	void OnResponseReady(void * imagePtr, int identifier) override;

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

