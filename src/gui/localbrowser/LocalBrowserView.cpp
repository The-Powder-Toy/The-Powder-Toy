#include <sstream>
#include "client/Client.h"
#include "Format.h"
#include "LocalBrowserView.h"
#include "PowderToy.h"

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
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	changed(false),
	lastChanged(0),
	pageCount(0)
{
	nextButton = new ui::Button(ui::Point(WINDOWW-52, WINDOWH-18), ui::Point(50, 16), "Next \x95");
	previousButton = new ui::Button(ui::Point(2, WINDOWH-18), ui::Point(50, 16), "\x96 Prev");
	undeleteButton = new ui::Button(ui::Point(WINDOWW-122, WINDOWH-18), ui::Point(60, 16), "Rescan");
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(undeleteButton);

	class PageNumAction : public ui::TextboxAction
	{
		LocalBrowserView * v;
	public:
		PageNumAction(LocalBrowserView * _v) { v = _v; }
		void TextChangedCallback(ui::Textbox * sender)
		{
			v->textChanged();
		}
	};
	pageTextbox = new ui::Textbox(ui::Point(283, WINDOWH-18), ui::Point(41, 16), "");
	pageTextbox->SetActionCallback(new PageNumAction(this));
	pageTextbox->SetInputType(ui::Textbox::Number);
	pageLabel = new ui::Label(ui::Point(0, WINDOWH-18), ui::Point(30, 16), "Page"); //page [TEXTBOX] of y
	pageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	pageCountLabel = new ui::Label(ui::Point(WINDOWW/2+6, WINDOWH-18), ui::Point(50, 16), "");
	pageCountLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(pageLabel);
	AddComponent(pageCountLabel);
	AddComponent(pageTextbox);

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

void LocalBrowserView::textChanged()
{
	int num = format::StringToNumber<int>(pageTextbox->GetText());
	if (num < 0) //0 is allowed so that you can backspace the 1
		pageTextbox->SetText("1");
	else if (num > pageCount)
		pageTextbox->SetText(format::NumberToString(pageCount));
	changed = true;
#ifdef USE_SDL
	lastChanged = GetTicks()+600;
#endif
}

void LocalBrowserView::OnTick(float dt)
{
	c->Update();
#ifdef USE_SDL
	if (changed && lastChanged < GetTicks())
	{
		changed = false;
		c->SetPage(std::max(format::StringToNumber<int>(pageTextbox->GetText()), 0));
	}
#endif
}

void LocalBrowserView::NotifyPageChanged(LocalBrowserModel * sender)
{
	pageCount = sender->GetPageCount();
	if (!sender->GetSavesList().size()) //no saves
	{
		pageLabel->Visible = pageCountLabel->Visible = pageTextbox->Visible = false;
	}
	else
	{
		std::stringstream pageInfo;
		pageInfo << "of " << pageCount;
		pageCountLabel->SetText(pageInfo.str());
		int width = Graphics::textwidth(pageInfo.str().c_str());

		pageLabel->Position.X = WINDOWW/2-width-20;
		pageTextbox->Position.X = WINDOWW/2-width+11;
		pageTextbox->Size.X = width-4;
		//pageCountLabel->Position.X = WINDOWW/2+6;
		pageLabel->Visible = pageCountLabel->Visible = pageTextbox->Visible = true;

		pageInfo.str("");
		pageInfo << sender->GetPageNum();
		pageTextbox->SetText(pageInfo.str());
	}

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
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 2;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<SaveFile*> saves = sender->GetSavesList();
	for (size_t i = 0; i < stampButtons.size(); i++)
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
	for (size_t i = 0; i < saves.size(); i++)
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
	for (size_t j = 0; j < stampButtons.size(); j++)
	{
		stampButtons[j]->SetSelected(false);
		for (size_t i = 0; i < selected.size(); i++)
		{
			if (stampButtons[j]->GetSaveFile()->GetName()==selected[i])
				stampButtons[j]->SetSelected(true);
		}
	}

	if (selected.size())
	{
		removeSelected->Visible = true;
		pageLabel->Visible = pageCountLabel->Visible = pageTextbox->Visible = false;
	}
	else if (removeSelected->Visible)
	{
		removeSelected->Visible = false;
		pageLabel->Visible = pageCountLabel->Visible = pageTextbox->Visible = true;
	}
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
	if (key == SDLK_ESCAPE)
		c->Exit();
	else if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->SetMoveToFront(false);
}

void LocalBrowserView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->SetMoveToFront(true);
}

LocalBrowserView::~LocalBrowserView() { }
