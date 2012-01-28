#include <iostream>

#include "SaveButton.h"
#include "search/Save.h"
#include "Graphics.h"
#include "Global.h"
#include "Engine.h"
#include "client/Client.h"

namespace ui {

SaveButton::SaveButton(Window* parent_state, Save * save):
	Component(parent_state),
	save(save),
	thumbnail(NULL),
	isMouseInside(false),
	isButtonDown(false),
	actionCallback(NULL)
{

}

SaveButton::SaveButton(Point position, Point size, Save * save):
	Component(position, size),
	save(save),
	thumbnail(NULL),
	isMouseInside(false),
	isButtonDown(false),
	actionCallback(NULL)
{

}

SaveButton::SaveButton(Save * save):
	Component(),
	save(save),
	thumbnail(NULL),
	isMouseInside(false),
	isButtonDown(false),
	actionCallback(NULL)
{

}

SaveButton::~SaveButton()
{
	if(thumbnail)
		delete thumbnail;
	if(actionCallback)
		delete actionCallback;
	if(save)
		delete save;
}

void SaveButton::Tick(float dt)
{
	Thumbnail * tempThumb;
	float scaleFactorY = 1.0f, scaleFactorX = 1.0f;
	if(!thumbnail)
	{
		tempThumb = Client::Ref().GetThumbnail(save->GetID(), 0);
		if(tempThumb)
		{
			thumbnail = new Thumbnail(*tempThumb); //Store a local copy of the thumbnail
			if(thumbnail->Data)
			{
				if(thumbnail->Size.Y > (Size.Y-25))
				{
					scaleFactorY = ((float)(Size.Y-25))/((float)thumbnail->Size.Y);
				}
				if(thumbnail->Size.X > Size.X)
				{
					scaleFactorX = ((float)Size.X)/((float)thumbnail->Size.X);
				}
				if(scaleFactorY < 1.0f || scaleFactorX < 1.0f)
				{
					float scaleFactor = scaleFactorY < scaleFactorX ? scaleFactorY : scaleFactorX;
					pixel * thumbData = thumbnail->Data;
					thumbnail->Data = Graphics::resample_img(thumbData, thumbnail->Size.X, thumbnail->Size.Y, thumbnail->Size.X * scaleFactor, thumbnail->Size.Y * scaleFactor);
					thumbnail->Size.X *= scaleFactor;
					thumbnail->Size.Y *= scaleFactor;
					free(thumbData);
				}
			}
		}
	}
}

void SaveButton::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	float scaleFactor;

	if(thumbnail)
	{
		g->draw_image(thumbnail->Data, screenPos.X+(Size.X-thumbnail->Size.X)/2, screenPos.Y+((Size.Y-25)-thumbnail->Size.Y)/2, thumbnail->Size.X, thumbnail->Size.Y, 255);
		g->drawrect(screenPos.X+(Size.X-thumbnail->Size.X)/2,  screenPos.Y+((Size.Y-25)-thumbnail->Size.Y)/2, thumbnail->Size.X, thumbnail->Size.Y, 180, 180, 180, 255);
	}
	else
	{
		scaleFactor = (Size.Y-25)/((float)YRES);
		g->drawrect(screenPos.X+(Size.X-((float)XRES)*scaleFactor)/2, screenPos.Y+((Size.Y-21)-((float)YRES)*scaleFactor)/2, ((float)XRES)*scaleFactor, ((float)YRES)*scaleFactor, 180, 180, 180, 255);
	}

	if(isMouseInside)
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->name.c_str()))/2, screenPos.Y+Size.Y - 21, save->name, 255, 255, 255, 255);
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->userName.c_str()))/2, screenPos.Y+Size.Y - 10, save->userName, 200, 230, 255, 255);
	}
	else
	{
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->name.c_str()))/2, screenPos.Y+Size.Y - 21, save->name, 180, 180, 180, 255);
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->userName.c_str()))/2, screenPos.Y+Size.Y - 10, save->userName, 100, 130, 160, 255);
	}
}

void SaveButton::OnMouseUnclick(int x, int y, unsigned int button)
{
	if(button != 1)
	{
		return; //left click only!
	}

	if(isButtonDown)
	{
		DoAction();
	}

	isButtonDown = false;
}

void SaveButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(button != 1) return; //left click only!
	isButtonDown = true;
}

void SaveButton::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
}

void SaveButton::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
}

void SaveButton::DoAction()
{
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void SaveButton::SetActionCallback(SaveButtonAction * action)
{
	actionCallback = action;
}

} /* namespace ui */
