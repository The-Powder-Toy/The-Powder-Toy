#include <iostream>
#include <typeinfo>

#include "SaveButton.h"
#include "client/SaveInfo.h"
#include "graphics/Graphics.h"
#include "Engine.h"
#include "client/ThumbnailBroker.h"
#include "simulation/SaveRenderer.h"

namespace ui {

SaveButton::SaveButton(Point position, Point size, SaveInfo * save):
	Component(position, size),
	file(NULL),
	save(save),
	thumbnail(NULL),
	isMouseInside(false),
	isButtonDown(false),
	actionCallback(NULL),
	voteColour(255, 0, 0),
	selectable(false),
	selected(false),
	waitingForThumb(false),
	isMouseInsideAuthor(false)
{
	if(save)
	{
		if(save->votesUp==0)
			voteRatio = 0.0f;
		else if(save->votesDown==0)
			voteRatio = 1.0f;
		else
			voteRatio = 1.0f-(float)(((float)(save->votesDown))/((float)(save->votesUp)));
		if(voteRatio < 0.0f)
			voteRatio = 0.0f;
		if(voteRatio > 1.0f)	//Not possible, but just in case the server were to give a negative value or something
			voteRatio = 1.0f;


		voteColour.Red = (1.0f-voteRatio)*255;
		voteColour.Green = voteRatio*255;
	}

	if(save)
	{
		name = save->name;
		if(Graphics::textwidth((char *)name.c_str()) > Size.X)
		{
			int position = Graphics::textwidthx((char *)name.c_str(), Size.X - 22);
			name = name.erase(position, name.length()-position);
			name += "...";
		}
	}
}

SaveButton::SaveButton(Point position, Point size, SaveFile * file):
	Component(position, size),
	save(NULL),
	file(file),
	thumbnail(NULL),
	isMouseInside(false),
	isButtonDown(false),
	actionCallback(NULL),
	voteColour(255, 0, 0),
	selectable(false),
	selected(false),
	wantsDraw(false),
	waitingForThumb(false),
	isMouseInsideAuthor(false)
{
	if(file)
	{
		name = file->GetDisplayName();
		if(Graphics::textwidth((char *)name.c_str()) > Size.X)
		{
			int position = Graphics::textwidthx((char *)name.c_str(), Size.X - 22);
			name = name.erase(position, name.length()-position);
			name += "...";
		}
	}
}

SaveButton::~SaveButton()
{
	ThumbnailBroker::Ref().DetachThumbnailListener(this);

	if(thumbnail)
		delete thumbnail;
	if(actionCallback)
		delete actionCallback;
	if(save)
		delete save;
	if(file)
		delete file;
}

void SaveButton::OnThumbnailReady(Thumbnail * thumb)
{
	if(thumb)
	{
		if(thumbnail)
			delete thumbnail;
		thumbnail = thumb;
		waitingForThumb = false;
	}
}

void SaveButton::Tick(float dt)
{
	if(!thumbnail && !waitingForThumb)
	{
		if(save)
		{
			if(save->GetGameSave())
			{
				waitingForThumb = true;
				ThumbnailBroker::Ref().RenderThumbnail(save->GetGameSave(), Size.X-3, Size.Y-25, this);
			}
			else if(save->GetID())
			{
				waitingForThumb = true;
				ThumbnailBroker::Ref().RetrieveThumbnail(save->GetID(), save->GetVersion(), Size.X-3, Size.Y-25, this);
			}
		}
		else if(file && file->GetGameSave())
		{
			waitingForThumb = true;
			ThumbnailBroker::Ref().RenderThumbnail(file->GetGameSave(), Size.X-3, Size.Y-25, this);
		}
	}
}

void SaveButton::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	float scaleFactor;
	ui::Point thumbBoxSize(0, 0);

	wantsDraw = true;

	if(selected && selectable)
	{
		g->fillrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 100, 170, 255, 100);
	}

	if(thumbnail)
	{
		thumbBoxSize = ui::Point(thumbnail->Size.X, thumbnail->Size.Y);
		if(save && save->id)
			g->draw_image(thumbnail->Data, screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbnail->Size.X, thumbnail->Size.Y, 255);
		else
			g->draw_image(thumbnail->Data, screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbnail->Size.X, thumbnail->Size.Y, 255);
	}
	else
	{
		scaleFactor = (Size.Y-25)/((float)YRES);
		thumbBoxSize = ui::Point(((float)XRES)*scaleFactor, ((float)YRES)*scaleFactor);
	}
	if(save)
	{
		if(save->id)
		{
			if(isMouseInside)
				g->drawrect(screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 210, 230, 255, 255);
			else
				g->drawrect(screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
			g->drawrect(screenPos.X-4+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, 7, thumbBoxSize.Y, 180, 180, 180, 255);

			int voteBar = std::max(10.0f, ((float)(thumbBoxSize.Y-4))*voteRatio);
			g->fillrect(1+screenPos.X-3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, (screenPos.Y-2)+(thumbBoxSize.Y-voteBar)+(Size.Y-21-thumbBoxSize.Y)/2, 3, voteBar, voteColour.Red, voteColour.Green, voteColour.Blue, 255);
		}
		else
		{
			if(isMouseInside)
				g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 210, 230, 255, 255);
			else
				g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
		}

		if(isMouseInside && !isMouseInsideAuthor)
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)name.c_str()))/2, screenPos.Y+Size.Y - 21, name, 255, 255, 255, 255);
		else
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)name.c_str()))/2, screenPos.Y+Size.Y - 21, name, 180, 180, 180, 255);

		if(isMouseInsideAuthor)
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->userName.c_str()))/2, screenPos.Y+Size.Y - 10, save->userName, 200, 230, 255, 255);
		else
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->userName.c_str()))/2, screenPos.Y+Size.Y - 10, save->userName, 100, 130, 160, 255);
	}
	if(file)
	{
		if(isMouseInside)
			g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 210, 230, 255, 255);
		else
			g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);

		if(isMouseInside)
		{
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)name.c_str()))/2, screenPos.Y+Size.Y - 21, name, 255, 255, 255, 255);
		}
		else
		{
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)name.c_str()))/2, screenPos.Y+Size.Y - 21, name, 180, 180, 180, 255);
		}
	}

	if(isMouseInside && selectable)
	{
		g->clearrect(screenPos.X+(Size.X-20), screenPos.Y+6, 14, 14);
		g->drawrect(screenPos.X+(Size.X-20), screenPos.Y+6, 14, 14, 255, 255, 255, 255);
		if(selected)
			g->fillrect(screenPos.X+(Size.X-18), screenPos.Y+8, 10, 10, 255, 255, 255, 255);
	}
}

void SaveButton::OnMouseUnclick(int x, int y, unsigned int button)
{
	if(button != 1)
	{
		return; //left click only!
	}

	if(x>=Size.X-20 && y>=6 && y<=20 && x<=Size.X-6 && selectable)
	{
		selected = !selected;
		DoSelection();
		return;
	}

	if(isButtonDown)
	{
		if(isMouseInsideAuthor)
			DoAuthorAction();
		else
			DoAction();
	}

	isButtonDown = false;
}

void SaveButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(button !=1 && selectable)
	{
		selected = !selected;
		DoSelection();
	}
	if(button != 1) return; //left click only!

	isButtonDown = true;
}

void SaveButton::OnMouseMovedInside(int x, int y, int dx, int dy)
{
	if(y > Size.Y-11)
	{
		isMouseInsideAuthor = true;
	}
	else
		isMouseInsideAuthor = false;
}

void SaveButton::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
}

void SaveButton::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
	isMouseInsideAuthor = false;
}

void SaveButton::DoAuthorAction()
{
	if(actionCallback)
		actionCallback->AuthorActionCallback(this);
}

void SaveButton::DoAction()
{
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void SaveButton::DoSelection()
{
	if(selectable)
		actionCallback->SelectedCallback(this);
}

void SaveButton::SetActionCallback(SaveButtonAction * action)
{
	actionCallback = action;
}

} /* namespace ui */
