#include "SaveButton.h"

#include "ContextMenu.h"
#include "Format.h"

#include "client/Client.h"
#include "client/ThumbnailRendererTask.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"

#include "gui/dialogues/ErrorMessage.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"

#include "SimulationConfig.h"
#include <SDL.h>

namespace ui {

SaveButton::SaveButton(Point position, Point size) :
	Component(position, size),
	wantsDraw(false),
	triedThumbnail(false),
	isMouseInsideAuthor(false),
	isMouseInsideHistory(false),
	showVotes(false),
	thumbnailRenderer(nullptr),
	isButtonDown(false),
	isMouseInside(false),
	selected(false),
	selectable(false)
{
}

SaveButton::SaveButton(Point position, Point size, SaveInfo *newSave /* non-owning */) : SaveButton(position, size)
{
	save = newSave;
	if(save)
	{
		name = save->name;
		if (Graphics::TextSize(name).X - 1 > Size.X)
		{
			auto it = Graphics::TextFit(name, Size.X - (Appearance.icon ? 38 : 22));
			name.erase(it, name.end());
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

SaveButton::SaveButton(Point position, Point size, SaveFile *newFile /* non-owning */) : SaveButton(position, size)
{
	file = newFile;
	if(file)
	{
		name = file->GetDisplayName();
		if (Graphics::TextSize(name).X - 1 > Size.X)
		{
			auto it = Graphics::TextFit(name, Size.X - (Appearance.icon ? 38 : 22));
			name.erase(it, name.end());
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
}

void SaveButton::Tick()
{
	if (!thumbnail)
	{
		if (!triedThumbnail && wantsDraw && ThumbnailRendererTask::QueueSize() < 10)
		{
			float scaleFactor = (Size.Y-25)/((float)YRES);
			ui::Point thumbBoxSize = ui::Point(int(XRES*scaleFactor), int(YRES*scaleFactor));
			if (save)
			{
				if(save->GetGameSave())
				{
					thumbnailRenderer = new ThumbnailRendererTask(*save->GetGameSave(), thumbBoxSize, RendererSettings::decorationEnabled, true);
					thumbnailRenderer->Start();
					triedThumbnail = true;
				}
				else if (save->GetID())
				{
					thumbnailRequest = std::make_unique<http::ThumbnailRequest>(save->GetID(), save->GetVersion(), thumbBoxSize);
					thumbnailRequest->Start();
					triedThumbnail = true;
				}
			}
			else if (file && file->GetGameSave())
			{
				thumbnailRenderer = new ThumbnailRendererTask(*file->GetGameSave(), thumbBoxSize, RendererSettings::decorationEnabled, false);
				thumbnailRenderer->Start();
				triedThumbnail = true;
			}
		}

		if (thumbnailRequest && thumbnailRequest->CheckDone())
		{
			try
			{
				thumbnail = thumbnailRequest->Finish();
			}
			catch (const http::RequestError &ex)
			{
				// TODO: handle
			}
			thumbnailRequest.reset();
		}

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
			thumbSize = thumbnail->Size();
		}
	}
	if (file && !wantsDraw && !thumbnailRenderer)
	{
		file->LazyUnload();
	}
	wantsDraw = false;
}

void SaveButton::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	float scaleFactor = (Size.Y-25)/((float)YRES);
	ui::Point thumbBoxSize = ui::Point(int(XRES*scaleFactor), int(YRES*scaleFactor));

	wantsDraw = true;

	if(selected && selectable)
	{
		g->BlendFilledRect(RectSized(screenPos, Size), 0x64AAFF_rgb .WithAlpha(100));
	}

	if (thumbnail)
	{
		//thumbBoxSize = ui::Point(thumbnail->Width, thumbnail->Height);
		auto *tex = thumbnail.get();
		auto space = Size - Vec2{ 0, 21 };
		g->BlendImage(tex->Data(), 255, RectSized(screenPos + ((save && save->id) ? ((space - thumbBoxSize) / 2 - Vec2{ 3, 0 }) : (space - thumbSize) / 2), tex->Size()));
	}
	else if (file && !file->LazyGetGameSave())
		g->BlendText(screenPos + Vec2{ (Size.X-(Graphics::TextSize("Error loading save").X - 1))/2, (Size.Y-28)/2 }, "Error loading save", 0xB4B4B4_rgb .WithAlpha(255));
	if(save)
	{
		if(save->id)
		{
			g->DrawRect(RectSized(screenPos + Vec2{                - 3, 0 } + (Size - thumbBoxSize - Vec2{ 0, 21 }) / 2,              thumbBoxSize), isMouseInside ? 0xD2E6FF_rgb : 0xB4B4B4_rgb);
			g->DrawRect(RectSized(screenPos + Vec2{ thumbBoxSize.X - 4, 0 } + (Size - thumbBoxSize - Vec2{ 0, 21 }) / 2, Vec2{ 7, thumbBoxSize.Y }), isMouseInside ? 0xD2E6FF_rgb : 0xB4B4B4_rgb);

			g->DrawFilledRect(RectSized(screenPos + Vec2{ -3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, 1+(Size.Y-20-thumbBoxSize.Y)/2 }, Vec2{ 5, (thumbBoxSize.Y+1)/2-1 }), 0x006B0A_rgb);
			g->DrawFilledRect(RectSized(screenPos + Vec2{ -3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, (Size.Y-20)/2 }, Vec2{ 5, thumbBoxSize.Y/2-1 }), 0x6B0A00_rgb);

			g->DrawFilledRect(RectSized(screenPos + Vec2{ -2+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, (Size.Y-20)/2-voteBarHeightUp }, Vec2{ 3, voteBarHeightUp }), 0x39BB39_rgb); //green
			g->DrawFilledRect(RectSized(screenPos + Vec2{ -2+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, (Size.Y-20)/2 }, Vec2{ 3, voteBarHeightDown }), 0xBB3939_rgb); //red
		}
		else
		{
			g->DrawRect(RectSized(screenPos + (Size - thumbBoxSize - Vec2{ 0, 21 }) / 2, thumbBoxSize), isMouseInside ? 0xD2E6FF_rgb : 0xB4B4B4_rgb);
		}

		g->BlendText(screenPos + Vec2{ (Size.X-(Graphics::TextSize(name).X - 1))/2, Size.Y - 21 }, name, (isMouseInside && !isMouseInsideAuthor) ? 0xFFFFFF_rgb .WithAlpha(255) : 0xB4B4B4_rgb .WithAlpha(255));
		g->BlendText(screenPos + Vec2{ (Size.X-(Graphics::TextSize(save->userName.FromUtf8()).X - 1))/2, Size.Y - 10 }, save->userName.FromUtf8(), isMouseInsideAuthor ? 0xC8E6FF_rgb .WithAlpha(255) : 0x6482A0_rgb .WithAlpha(255));
		if (showVotes)// && !isMouseInside)
		{
			int x = screenPos.X-7+(Size.X-thumbBoxSize.X)/2+thumbBoxSize.X-(Graphics::TextSize(votesBackground).X - 1);
			int y = screenPos.Y-23+(Size.Y-thumbBoxSize.Y)/2+thumbBoxSize.Y;
			g->BlendText({ x, y }, votesBackground, 0x104810_rgb .WithAlpha(255));
			g->BlendText({ x, y }, votesBackground2, 0xC0C0C0_rgb .WithAlpha(255));
			g->BlendText({ x+3, y }, votesString, 0xFFFFFF_rgb .WithAlpha(255));
		}
		if (isMouseInside)
		{
			int x = screenPos.X;
			int y = screenPos.Y-15+(Size.Y-thumbBoxSize.Y)/2+thumbBoxSize.Y;
			g->DrawFilledRect(RectSized(Vec2{ x+1, y+1 }, Vec2{ 7, 8 }), 0xFFFFFF_rgb);
			g->BlendText({ x, y }, 0xE026, isMouseInsideHistory ? 0xC86450_rgb .WithAlpha(255) : 0xA04632_rgb .WithAlpha(255));
		}
		if (!save->GetPublished())
		{
			g->BlendText(screenPos - Vec2{ 0, 2 }, 0xE04D, 0xFFFFFF_rgb .WithAlpha(255));
			g->BlendText(screenPos - Vec2{ 0, 2 }, 0xE04E, 0xD49751_rgb .WithAlpha(255));
		}
	}
	else if (file)
	{
		g->DrawRect(RectSized(screenPos + (Size - thumbBoxSize- Vec2{ 0, 21 }) / 2, thumbBoxSize), isMouseInside ? 0xD2E6FF_rgb : 0xB4B4B4_rgb);
		if (thumbSize.X)
			g->XorDottedRect(RectSized(screenPos + (Size - thumbSize - Vec2{ 0, 21 }) / 2, thumbSize));

		g->BlendText(screenPos + Vec2{ (Size.X-(Graphics::TextSize(name).X - 1))/2, Size.Y - 21 }, name, isMouseInside ? 0xFFFFFF_rgb .WithAlpha(255) : 0xB4B4B4_rgb .WithAlpha(255));
	}

	if(isMouseInside && selectable)
	{
		g->DrawFilledRect(RectSized(screenPos + Vec2{ Size.X - 19, 7 }, Vec2{ 13, 13 }), 0x000000_rgb);
		g->DrawRect(RectSized(screenPos + Vec2{ Size.X-20, 6 }, Vec2{ 14, 14 }), 0xFFFFFF_rgb);
		if(selected)
			g->DrawFilledRect(RectSized(screenPos + Vec2{ Size.X-18, 8 }, Vec2{ 10, 10 }), 0xFFFFFF_rgb);
	}
}

void SaveButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(button != 1)
	{
		return; //left click only!
	}
	if (file && !file->LazyGetGameSave())
	{
		new ErrorMessage("Error loading save", file->GetError());
		return;
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

void SaveButton::OnMouseDown(int x, int y, unsigned int button)
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
			if(button !=1 && selectable)
			{
				selected = !selected;
				DoSelection();
			}

		}
	}
}

void SaveButton::OnMouseMoved(int x, int y)
{
	isMouseInsideAuthor = false;
	isMouseInsideHistory = false;
	if (MouseInside)
	{
		if (y > Size.Y-11)
			isMouseInsideAuthor = true;

		if (y > Size.Y-29 && y < Size.Y - 18 && x > 0 && x < 9)
			isMouseInsideHistory = true;
	}
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
	if (actionCallback.altAction)
		actionCallback.altAction();
}

void SaveButton::DoAltAction2()
{
	if (actionCallback.altAltAction)
		actionCallback.altAltAction();
}

void SaveButton::DoAction()
{
	if (actionCallback.action)
		actionCallback.action();
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
	if (selectable && actionCallback.selected)
		actionCallback.selected();
}

std::unique_ptr<VideoBuffer> SaveButton::CloneThumbnail() const
{
	if (thumbnail)
	{
		return std::make_unique<VideoBuffer>(*thumbnail);
	}
	return nullptr;
}

} /* namespace ui */
