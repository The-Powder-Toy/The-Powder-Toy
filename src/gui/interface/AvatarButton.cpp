#include <iostream>
#include <typeinfo>

#include "AvatarButton.h"
#include "Format.h"
#include "client/Client.h"
#include "graphics/Graphics.h"
#include "ContextMenu.h"
#include "Keys.h"
#include "Mouse.h"

namespace ui {

AvatarButton::AvatarButton(Point position, Point size, ByteString username):
	Component(position, size),
	name(username),
	tried(false),
	actionCallback(NULL)
{

}

AvatarButton::~AvatarButton()
{
	delete actionCallback;
}

void AvatarButton::OnResponse(std::unique_ptr<VideoBuffer> Avatar)
{
	avatar = std::move(Avatar);
}

void AvatarButton::Tick(float dt)
{
	if(!avatar && !tried && name.size() > 0)
	{
		tried = true;
		RequestSetup(name, Size.X, Size.Y);
		RequestStart();
	}

	RequestPoll();
}

void AvatarButton::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();

	if(avatar)
	{
		g->draw_image(avatar.get(), screenPos.X, screenPos.Y, 255);
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
	if(button == SDL_BUTTON_RIGHT)
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
