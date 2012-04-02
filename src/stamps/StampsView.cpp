/*
 * StampsView.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#include <sstream>
#include "client/Client.h"
#include "StampsView.h"

#include "interface/SaveButton.h"
#include "dialogues/ErrorMessage.h"
#include "StampsController.h"
#include "StampsModel.h"
#include "StampsModelException.h"

StampsView::StampsView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE))
{
	nextButton = new ui::Button(ui::Point(XRES+BARSIZE-52, YRES+MENUSIZE-18), ui::Point(50, 16), "Next \x95");
	previousButton = new ui::Button(ui::Point(1, YRES+MENUSIZE-18), ui::Point(50, 16), "\x96 Prev");
	infoLabel  = new ui::Label(ui::Point(51, YRES+MENUSIZE-18), ui::Point(XRES+BARSIZE-102, 16), "Loading...");
	AddComponent(infoLabel);
	AddComponent(nextButton);
	AddComponent(previousButton);

	class NextPageAction : public ui::ButtonAction
	{
		StampsView * v;
	public:
		NextPageAction(StampsView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->NextPage();
		}
	};
	nextButton->SetActionCallback(new NextPageAction(this));
	nextButton->SetAlignment(AlignRight, AlignBottom);

	class PrevPageAction : public ui::ButtonAction
	{
		StampsView * v;
	public:
		PrevPageAction(StampsView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->PrevPage();
		}
	};
	previousButton->SetActionCallback(new PrevPageAction(this));
	previousButton->SetAlignment(AlignLeft, AlignBottom);
}

void StampsView::OnTick(float dt)
{
	c->Update();
}

void StampsView::NotifyPageChanged(StampsModel * sender)
{
	std::stringstream pageInfo;
	pageInfo << "Page " << sender->GetPageNum() << " of " << sender->GetPageCount();
	infoLabel->SetText(pageInfo.str());
	if(sender->GetPageNum() == 1)
	{
		previousButton->Visible = false;
	}
	else
	{
		previousButton->Visible = true;
	}
	if(sender->GetPageNum() == sender->GetPageCount())
	{
		nextButton->Visible = false;
	}
	else
	{
		nextButton->Visible = true;
	}
}

void StampsView::NotifyStampsListChanged(StampsModel * sender)
{
	int i = 0;
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 2;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<Save*> saves = sender->GetStampsList();
	Client::Ref().ClearThumbnailRequests();
	for(i = 0; i < stampButtons.size(); i++)
	{
		RemoveComponent(stampButtons[i]);
		delete stampButtons[i];
	}

	buttonXOffset = 0;
	buttonYOffset = 50;
	buttonAreaWidth = Size.X;
	buttonAreaHeight = Size.Y - buttonYOffset - 18;
	buttonWidth = (buttonAreaWidth/savesX) - buttonPadding*2;
	buttonHeight = (buttonAreaHeight/savesY) - buttonPadding*2;
	class SaveOpenAction: public ui::SaveButtonAction
	{
		StampsView * v;
	public:
		SaveOpenAction(StampsView * _v) { v = _v; }
		virtual void ActionCallback(ui::SaveButton * sender)
		{
			v->c->OpenStamp(sender->GetSave());
		}
	};
	for(i = 0; i < saves.size(); i++)
	{
		if(saveX == savesX)
		{
			if(saveY == savesY-1)
				break;
			saveX = 0;
			saveY++;
		}
		ui::SaveButton * saveButton;
		saveButton = new ui::SaveButton(
					ui::Point(
						buttonXOffset + buttonPadding + saveX*(buttonWidth+buttonPadding*2),
						buttonYOffset + buttonPadding + saveY*(buttonHeight+buttonPadding*2)
						),
					ui::Point(buttonWidth, buttonHeight),
					saves[i]);
		saveButton->SetActionCallback(new SaveOpenAction(this));
		stampButtons.push_back(saveButton);
		AddComponent(saveButton);
		saveX++;
	}
}

StampsView::~StampsView() {
	// TODO Auto-generated destructor stub
}

