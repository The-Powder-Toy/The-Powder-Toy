#ifndef AVATARBUTTON_H_
#define AVATARBUTTON_H_

#include <string>

#include "Component.h"
#include "graphics/Graphics.h"
#include "interface/Colour.h"

namespace ui
{
class AvatarButton;
class AvatarButtonAction
{
public:
	virtual void ActionCallback(ui::AvatarButton * sender) {}
	virtual ~AvatarButtonAction() {}
};

class AvatarButton : public Component
{
	VideoBuffer * avatar;
	std::string name;
	bool tried;
public:
	AvatarButton(Point position, Point size, std::string username);
	virtual ~AvatarButton();

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void OnContextMenuAction(int item);

	virtual void Draw(const Point& screenPos);
	virtual void Tick(float dt);

	virtual void DoAction();

	void SetUsername(std::string username) { name = username; }
	std::string GetUsername() { return name; }
	void SetActionCallback(AvatarButtonAction * action);
protected:
	bool isMouseInside, isButtonDown;
	AvatarButtonAction * actionCallback;
};
}
#endif /* AVATARBUTTON_H_ */

