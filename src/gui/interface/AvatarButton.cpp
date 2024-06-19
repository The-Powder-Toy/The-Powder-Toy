#include "Button.h"
#include "AvatarButton.h"
#include "Format.h"
#include "graphics/Graphics.h"
#include "ContextMenu.h"
#include "Config.h"
#include <iostream>
#include <SDL.h>

namespace ui {

AvatarButton::AvatarButton(Point position, Point size, ByteString username):
	Component(position, size),
	name(username),
	tried(false)
{

}

void AvatarButton::Tick(float dt)
{
	if (!avatar && !tried && name.size() > 0)
	{
		tried = true;
		imageRequest = std::make_unique<http::ImageRequest>(ByteString::Build(STATICSERVER, "/avatars/", name, ".png"), Size);
		imageRequest->Start();
	}

	if (imageRequest && imageRequest->CheckDone())
	{
		try
		{
			avatar = imageRequest->Finish();
		}
		catch (const http::RequestError &ex)
		{
			// Nothing, oh well.
		}
		imageRequest.reset();
	}
}

void AvatarButton::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();

	if(avatar)
	{
		auto *tex = avatar.get();
		g->BlendImage(tex->Data(), 255, RectSized(screenPos, tex->Size()));
	}
}

void AvatarButton::OnMouseClick(int x, int y, unsigned int button)
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

void AvatarButton::OnMouseDown(int x, int y, unsigned int button)
{
	if (MouseDownInside)
	{
		if(button == SDL_BUTTON_RIGHT)
		{
			if(menu)
				menu->Show(GetContainerPos() + ui::Point(x, y));
		}
		else
		{
			isButtonDown = true;
		}
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
	if( actionCallback.action)
		actionCallback.action();
}

} /* namespace ui */
