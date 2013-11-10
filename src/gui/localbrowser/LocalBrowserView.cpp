#include <sstream>
#include "client/Client.h"
#include "LocalBrowserView.h"

#include "gui/interface/Button.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/Keys.h"
 
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "LocalBrowserController.h"
#include "LocalBrowserModel.h"
#include "LocalBrowserModelException.h"

LocalBrowserView::LocalBrowserView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH))
{
	nextButton = new ui::Button(ui::Point(WINDOWW-52, WINDOWH-18), ui::Point(50, 16), "Next \x95");
	previousButton = new ui::Button(ui::Point(1, WINDOWH-18), ui::Point(50, 16), "\x96 Prev");
	undeleteButton = new ui::Button(ui::Point(WINDOWW-122, WINDOWH-18), ui::Point(60, 16), "Rescan");
	infoLabel  = new ui::Label(ui::Point(51, WINDOWH-18), ui::Point(WINDOWW-102, 16), "Loading...");
	AddComponent(infoLabel);
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(undeleteButton);

	class NextPageAction : public ui::ButtonAction
	{
		LocalBrowserView * v;
	public:
		NextPageAction(LocalBrowserView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->NextPage();
		}
	};
	nextButton->SetActionCallback(new NextPageAction(this));
	nextButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	nextButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;

	class PrevPageAction : public ui::ButtonAction
	{
		LocalBrowserView * v;
	public:
		PrevPageAction(LocalBrowserView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->PrevPage();
		}
	};
	previousButton->SetActionCallback(new PrevPageAction(this));
	previousButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previousButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;

	class UndeleteAction : public ui::ButtonAction
	{
		LocalBrowserView * v;
	public:
		UndeleteAction(LocalBrowserView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->RescanStamps();
		}
	};
	undeleteButton->SetActionCallback(new UndeleteAction(this));

	class RemoveSelectedAction : public ui::ButtonAction
	{
		LocalBrowserView * v;
	public:
		RemoveSelectedAction(LocalBrowserView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->RemoveSelected();
		}
	};

	removeSelected = new ui::Button(ui::Point(((WINDOWW-100)/2), WINDOWH-18), ui::Point(100, 16), "Delete");
	removeSelected->Visible = false;
	removeSelected->SetActionCallback(new RemoveSelectedAction(this));
	AddComponent(removeSelected);
}

void LocalBrowserView::OnTick(float dt)
{
	c->Update();
}

void LocalBrowserView::NotifyPageChanged(LocalBrowserModel * sender)
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

void LocalBrowserView::NotifySavesListChanged(LocalBrowserModel * sender)
{
	int i = 0;
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 2;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<SaveFile*> saves = sender->GetSavesList();
	for(i = 0; i < stampButtons.size(); i++)
	{
		RemoveComponent(stampButtons[i]);
		delete stampButtons[i];
	}
	stampButtons.clear();
	buttonXOffset = 0;
	buttonYOffset = 50;
	buttonAreaWidth = Size.X;
	buttonAreaHeight = Size.Y - buttonYOffset - 18;
	buttonWidth = (buttonAreaWidth/savesX) - buttonPadding*2;
	buttonHeight = (buttonAreaHeight/savesY) - buttonPadding*2;
	class SaveOpenAction: public ui::SaveButtonAction
	{
		LocalBrowserView * v;
	public:
		SaveOpenAction(LocalBrowserView * _v) { v = _v; }
		virtual void ActionCallback(ui::SaveButton * sender)
		{
			if(sender->GetSaveFile())
				v->c->OpenSave(sender->GetSaveFile());
		}
		virtual void SelectedCallback(ui::SaveButton * sender)
		{
			if(sender->GetSaveFile())
				v->c->Selected(sender->GetSaveFile()->GetName(), sender->GetSelected());
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
		saveButton->SetSelectable(true);
		saveButton->SetActionCallback(new SaveOpenAction(this));
		stampButtons.push_back(saveButton);
		AddComponent(saveButton);
		saveX++;
	}
}

void LocalBrowserView::NotifySelectedChanged(LocalBrowserModel * sender)
{
	vector<std::string> selected = sender->GetSelected();
	for(int j = 0; j < stampButtons.size(); j++)
	{
		stampButtons[j]->SetSelected(false);
		for(int i = 0; i < selected.size(); i++)
		{
			if(stampButtons[j]->GetSaveFile()->GetName()==selected[i])
				stampButtons[j]->SetSelected(true);
		}
	}

	if(selected.size())
	{
		removeSelected->Visible = true;
	}
	else
		removeSelected->Visible = false;
}

void LocalBrowserView::OnMouseWheel(int x, int y, int d)
{
	if(!d)
		return;
	if(d<0)
		c->NextPage();
	else
		c->PrevPage();
}

void LocalBrowserView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(key == KEY_ESCAPE)
		c->Exit();
	else if (key == KEY_LCTRL || key == KEY_RCTRL)
		c->SetMoveToFront(false);
}

void LocalBrowserView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (key == KEY_LCTRL || key == KEY_RCTRL)
		c->SetMoveToFront(true);
}

LocalBrowserView::~LocalBrowserView() {
}

