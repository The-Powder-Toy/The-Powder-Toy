#include <iostream>
#include <typeinfo>

#include "ContextMenu.h"
#include "Format.h"
#include "Keys.h"
#include "Mouse.h"
#include "SaveButton.h"
#include "client/Client.h"
#include "client/SaveInfo.h"
#include "client/ThumbnailRendererTask.h"
#include "simulation/SaveRenderer.h"
#include "client/GameSave.h"
#include "simulation/SaveRenderer.h"

namespace ui {

SaveButton::SaveButton(Point position, Point size, SaveInfo * save):
	Component(position, size),
	file(NULL),
	save(save),
	triedThumbnail(false),
	isMouseInsideAuthor(false),
	isMouseInsideHistory(false),
	showVotes(false),
	thumbnailRenderer(nullptr),
	isButtonDown(false),
	isMouseInside(false),
	selected(false),
	selectable(false),
	actionCallback(NULL)
{
	if(save)
	{
		name = save->name;
		if(Graphics::textwidth(name) > Size.X)
		{
			int position = Graphics::textwidthx(name, Size.X - 22);
			name = name.erase(position, name.length()-position);
			name += "...";
		}

		String votes, icon;

		votes = String::Build(save->GetVotesUp()-save->GetVotesDown());
		icon += 0xE03B;
		for (size_t j = 1; j < votes.length(); j++)
			icon += 0xE03C;
		icon += 0xE039;
		icon += 0xE03A;

		votesBackground = icon;

		for (String::iterator iter = icon.begin(), end = icon.end(); iter != end; ++iter)
			*iter -= 14; // 0xE039 -> 0xE02B

		votesBackground2 = icon;

		for (String::iterator iter = votes.begin(), end = votes.end(); iter != end; ++iter)
			if(*iter != '-')
				*iter += 0xDFFF; // 0x30 -> 0xE02F

		votesString = votes;

		int voteMax = std::max(save->GetVotesUp(),save->GetVotesDown());
		if (voteMax)
		{
			if (voteMax < 34)
			{
				float ry = 33.0f/voteMax;
				if (voteMax<8)
					ry =  ry/(8-voteMax);
				voteBarHeightUp = (int)(save->GetVotesUp()*ry)-1;
				voteBarHeightDown = (int)(save->GetVotesDown()*ry)-1;
			}
			else
			{
				float ry = voteMax/33.0f;
				voteBarHeightUp = (int)(save->GetVotesUp()/ry)-1;
				voteBarHeightDown = (int)(save->GetVotesDown()/ry)-1;
			}
		}
		else
		{
			voteBarHeightUp = 0;
			voteBarHeightDown = 0;
		}
	}
}

SaveButton::SaveButton(Point position, Point size, SaveFile * file):
	Component(position, size),
	file(file),
	save(NULL),
	wantsDraw(false),
	triedThumbnail(false),
	isMouseInsideAuthor(false),
	isMouseInsideHistory(false),
	showVotes(false),
	thumbnailRenderer(nullptr),
	isButtonDown(false),
	isMouseInside(false),
	selected(false),
	selectable(false),
	actionCallback(NULL)
{
	if(file)
	{
		name = file->GetDisplayName();
		if(Graphics::textwidth(name) > Size.X)
		{
			int position = Graphics::textwidthx(name, Size.X - 22);
			name = name.erase(position, name.length()-position);
			name += "...";
		}
	}
}

SaveButton::~SaveButton()
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Abandon();
	}
	delete actionCallback;
	delete save;
	delete file;
}

void SaveButton::OnResponse(std::unique_ptr<VideoBuffer> Thumbnail)
{
	thumbnail = std::move(Thumbnail);
}

void SaveButton::Tick(float dt)
{
	if (!thumbnail)
	{
		if (!triedThumbnail)
		{
			float scaleFactor = (Size.Y-25)/((float)YRES);
			ui::Point thumbBoxSize = ui::Point(((float)XRES)*scaleFactor, ((float)YRES)*scaleFactor);
			if (save)
			{
				if(save->GetGameSave())
				{
					thumbnailRenderer = new ThumbnailRendererTask(save->GetGameSave(), thumbBoxSize.X, thumbBoxSize.Y);
					thumbnailRenderer->Start();
					triedThumbnail = true;
				}
				else if (save->GetID())
				{
					RequestSetup(save->GetID(), save->GetVersion(), thumbBoxSize.X, thumbBoxSize.Y);
					RequestStart();
					triedThumbnail = true;
				}
			}
			else if (file && file->GetGameSave())
			{
				thumbnailRenderer = new ThumbnailRendererTask(file->GetGameSave(), thumbBoxSize.X, thumbBoxSize.Y, true, true, false);
				thumbnailRenderer->Start();
				triedThumbnail = true;
			}
		}

		RequestPoll();

		if (thumbnailRenderer)
		{
			thumbnailRenderer->Poll();
			if (thumbnailRenderer->GetDone())
			{
				thumbnail = thumbnailRenderer->Finish();
				thumbnailRenderer = nullptr;
			}
		}

		if (thumbnail && file)
		{
			thumbSize = ui::Point(thumbnail->Width, thumbnail->Height);
		}
	}
}

void SaveButton::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	float scaleFactor = (Size.Y-25)/((float)YRES);
	ui::Point thumbBoxSize = ui::Point(((float)XRES)*scaleFactor, ((float)YRES)*scaleFactor);

	wantsDraw = true;

	if(selected && selectable)
	{
		g->fillrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 100, 170, 255, 100);
	}

	if (thumbnail)
	{
		//thumbBoxSize = ui::Point(thumbnail->Width, thumbnail->Height);
		if (save && save->id)
			g->draw_image(thumbnail.get(), screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, 255);
		else
			g->draw_image(thumbnail.get(), screenPos.X+(Size.X-thumbSize.X)/2, screenPos.Y+(Size.Y-21-thumbSize.Y)/2, 255);
	}
	else if (file && !file->GetGameSave())
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth("Error loading save"))/2, screenPos.Y+(Size.Y-28)/2, "Error loading save", 180, 180, 180, 255);
	if(save)
	{
		if(save->id)
		{
			if(isMouseInside)
			{
				g->drawrect(screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 210, 230, 255, 255);
				g->drawrect(screenPos.X-4+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, 7, thumbBoxSize.Y, 210, 230, 255, 255);
			}
			else
			{
				g->drawrect(screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
				g->drawrect(screenPos.X-4+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, 7, thumbBoxSize.Y, 180, 180, 180, 255);
			}

			g->fillrect(screenPos.X-3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+1+(Size.Y-20-thumbBoxSize.Y)/2, 5, (thumbBoxSize.Y+1)/2-1, 0, 107, 10, 255);
			g->fillrect(screenPos.X-3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-20)/2, 5, thumbBoxSize.Y/2-1, 107, 10, 0, 255);

			g->fillrect(screenPos.X-2+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-20)/2-voteBarHeightUp, 3, voteBarHeightUp, 57, 187, 57, 255); //green
			g->fillrect(screenPos.X-2+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-20)/2, 3, voteBarHeightDown, 187, 57, 57, 255); //red
		}
		else
		{
			if(isMouseInside)
				g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 210, 230, 255, 255);
			else
				g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
		}

		if(isMouseInside && !isMouseInsideAuthor)
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth(name))/2, screenPos.Y+Size.Y - 21, name, 255, 255, 255, 255);
		else
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth(name))/2, screenPos.Y+Size.Y - 21, name, 180, 180, 180, 255);

		if(isMouseInsideAuthor)
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth(save->userName.FromUtf8()))/2, screenPos.Y+Size.Y - 10, save->userName.FromUtf8(), 200, 230, 255, 255);
		else
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth(save->userName.FromUtf8()))/2, screenPos.Y+Size.Y - 10, save->userName.FromUtf8(), 100, 130, 160, 255);
		if (showVotes)// && !isMouseInside)
		{
			int x = screenPos.X-7+(Size.X-thumbBoxSize.X)/2+thumbBoxSize.X-Graphics::textwidth(votesBackground);
			int y = screenPos.Y-23+(Size.Y-thumbBoxSize.Y)/2+thumbBoxSize.Y;
			g->drawtext(x, y, votesBackground, 16, 72, 16, 255);
			g->drawtext(x, y, votesBackground2, 192, 192, 192, 255);
			g->drawtext(x+3, y, votesString, 255, 255, 255, 255);
		}
		if (isMouseInsideHistory && showVotes)
		{
			int x = screenPos.X;
			int y = screenPos.Y-15+(Size.Y-thumbBoxSize.Y)/2+thumbBoxSize.Y;
			g->fillrect(x+1, y+1, 7, 8, 255, 255, 255, 255);
			if (isMouseInsideHistory) {
				g->drawtext(x, y, 0xE026, 200, 100, 80, 255);
			} else {
				g->drawtext(x, y, 0xE026, 160, 70, 50, 255);
			}
		}
		if (!save->GetPublished())
		{
			g->drawtext(screenPos.X, screenPos.Y-2, 0xE04D, 255, 255, 255, 255);
			g->drawtext(screenPos.X, screenPos.Y-2, 0xE04E, 212, 151, 81, 255);
		}
	}
	else if (file)
	{
		if (isMouseInside)
			g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 210, 230, 255, 255);
		else
			g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
		if (thumbSize.X)
			g->xor_rect(screenPos.X+(Size.X-thumbSize.X)/2, screenPos.Y+(Size.Y-21-thumbSize.Y)/2, thumbSize.X, thumbSize.Y);

		if (isMouseInside)
		{
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth(name))/2, screenPos.Y+Size.Y - 21, name, 255, 255, 255, 255);
		}
		else
		{
			g->drawtext(screenPos.X+(Size.X-Graphics::textwidth(name))/2, screenPos.Y+Size.Y - 21, name, 180, 180, 180, 255);
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
		isButtonDown = false;
		if(isMouseInsideHistory)
			DoAltAction();
		else if(isMouseInsideAuthor)
			DoAltAction2();
		else
			DoAction();
	}
}

void SaveButton::AddContextMenu(int menuType)
{
	if (menuType == 0) //Save browser
	{
		menu = new ContextMenu(this);
		menu->AddItem(ContextMenuItem("Open", 0, true));
		if (Client::Ref().GetAuthUser().UserID)
			menu->AddItem(ContextMenuItem("Select", 1, true));
		menu->AddItem(ContextMenuItem("View History", 2, true));
		menu->AddItem(ContextMenuItem("More by this user", 3, true));
	}
	else if (menuType == 1) //Local save browser
	{
		menu = new ContextMenu(this);
		menu->AddItem(ContextMenuItem("Open", 0, true));
		menu->AddItem(ContextMenuItem("Rename", 2, true));
		menu->AddItem(ContextMenuItem("Delete", 3, true));
	}
}

void SaveButton::OnContextMenuAction(int item)
{
	switch(item)
	{
	case 0:
		DoAction();
		break;
	case 1:
		selected = !selected;
		DoSelection();
		break;
	case 2:
		DoAltAction();
		break;
	case 3:
		DoAltAction2();
		break;
	}
}

void SaveButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(button == SDL_BUTTON_RIGHT)
	{
		if(menu)
			menu->Show(GetScreenPos() + ui::Point(x, y));
	}
	else
	{
		isButtonDown = true;
		if(button !=1 && selectable)
		{
			selected = !selected;
			DoSelection();
		}

	}
}

void SaveButton::OnMouseMovedInside(int x, int y, int dx, int dy)
{
	if(y > Size.Y-11)
		isMouseInsideAuthor = true;
	else
		isMouseInsideAuthor = false;

	if(showVotes && y > Size.Y-29 && y < Size.Y - 18 && x > 0 && x < 9)
		isMouseInsideHistory = true;
	else
		isMouseInsideHistory = false;
}

void SaveButton::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
}

void SaveButton::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
	isMouseInsideAuthor = false;
	isMouseInsideHistory = false;
}

void SaveButton::DoAltAction()
{
	if(actionCallback)
		actionCallback->AltActionCallback(this);
}

void SaveButton::DoAltAction2()
{
	if(actionCallback)
		actionCallback->AltActionCallback2(this);
}

void SaveButton::DoAction()
{
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void SaveButton::DoSelection()
{
	if(menu)
	{
		if(selected)
			menu->SetItem(1, "Deselect");
		else
			menu->SetItem(1, "Select");
	}
	if(selectable && actionCallback)
		actionCallback->SelectedCallback(this);
}

void SaveButton::SetActionCallback(SaveButtonAction * action)
{
	actionCallback = action;
}

} /* namespace ui */
