#include <iostream>
#include <typeinfo>

#include "AvatarButton.h"
#include "Format.h"
#include "Engine.h"
#include "client/Client.h"
#include "client/requestbroker/RequestBroker.h"
#include "graphics/Graphics.h"
#include "ContextMenu.h"
#include "Keys.h"

namespace ui {

AvatarButton::AvatarButton(Point position, Point size, std::string username):
	Component(position, size),
	name(username),
	actionCallback(NULL),
	avatar(NULL),
	tried(false)
{

}

AvatarButton::~AvatarButton()
{
	RequestBroker::Ref().DetachRequestListener(this);
	if(avatar)
		delete avatar;
	if(actionCallback)
		delete actionCallback;
}

void AvatarButton::Tick(float dt)
{
	if(!avatar && !tried && name.size() > 0)
	{
		tried = true;
		RequestBroker::Ref().RetrieveAvatar(name, Size.X, Size.Y, this);
	}
}

void AvatarButton::OnResponseReady(void * imagePtr)
{
	VideoBuffer * image = (VideoBuffer*)imagePtr;
	if(image)
	{
		if(avatar)
			delete avatar;
		avatar = image;
	}
}

void AvatarButton::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;

	if(avatar)
	{
		g->draw_image(avatar, screenPos.X, screenPos.Y, 255);
	}
}

void AvatarButton::OnMouseUnclick(int x, int y, unsigned int button)
{
	if(button != 1)
	{
		return; //left click only!
	}

	if(isButtonDown)
	{
		isButtonDown = false;
		DoAction();
	}
}

void AvatarButton::OnContextMenuAction(int item)
{
	//Do nothing
}

void AvatarButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(button == BUTTON_RIGHT)
	{
		if(menu)
			menu->Show(GetScreenPos() + ui::Point(x, y));
	}
	else
	{
		isButtonDown = true;
	}
}

void AvatarButton::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
}

void AvatarButton::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
}

void AvatarButton::DoAction()
{
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void AvatarButton::SetActionCallback(AvatarButtonAction * action)
{
	actionCallback = action;
}

} /* namespace ui */
