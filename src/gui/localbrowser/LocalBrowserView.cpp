#include "LocalBrowserView.h"

#include "LocalBrowserController.h"
#include "LocalBrowserModel.h"

#include "gui/interface/Button.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/Keys.h"

#include "PowderToy.h"
#include "Config.h"

#include "client/SaveFile.h"

#include "graphics/Graphics.h"

LocalBrowserView::LocalBrowserView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	changed(false),
	lastChanged(0),
	pageCount(0)
{
	nextButton = new ui::Button(ui::Point(WINDOWW-52, WINDOWH-18), ui::Point(50, 16), String("Next ") + 0xE015);
	previousButton = new ui::Button(ui::Point(2, WINDOWH-18), ui::Point(50, 16), 0xE016 + String(" Prev"));
	undeleteButton = new ui::Button(ui::Point(WINDOWW-122, WINDOWH-18), ui::Point(60, 16), "Rescan");
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(undeleteButton);

	pageTextbox = new ui::Textbox(ui::Point(283, WINDOWH-18), ui::Point(41, 16), "");
	pageTextbox->SetActionCallback({ [this] { textChanged(); } });
	pageTextbox->SetInputType(ui::Textbox::Number);
	pageLabel = new ui::Label(ui::Point(0, WINDOWH-18), ui::Point(30, 16), "Page"); //page [TEXTBOX] of y
	pageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	pageCountLabel = new ui::Label(ui::Point(WINDOWW/2+6, WINDOWH-18), ui::Point(50, 16), "");
	pageCountLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(pageLabel);
	AddComponent(pageCountLabel);
	AddComponent(pageTextbox);

	nextButton->SetActionCallback({ [this] { c->SetPageRelative(1); } });
	nextButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	nextButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;

	previousButton->SetActionCallback({ [this] { c->SetPageRelative(-1); } });
	previousButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previousButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;

	undeleteButton->SetActionCallback({ [this] { c->RescanStamps(); } });

	removeSelected = new ui::Button(ui::Point(((WINDOWW-100)/2), WINDOWH-18), ui::Point(100, 16), "Delete");
	removeSelected->Visible = false;
	removeSelected->SetActionCallback({ [this] { c->RemoveSelected(); } });
	AddComponent(removeSelected);
}

void LocalBrowserView::textChanged()
{
	int num = pageTextbox->GetText().ToNumber<int>(true);
	if (num < 0) //0 is allowed so that you can backspace the 1
		pageTextbox->SetText("1");
	else if (num > pageCount)
		pageTextbox->SetText(String::Build(pageCount));
	changed = true;
	lastChanged = GetTicks()+600;
}

void LocalBrowserView::OnTick(float dt)
{
	c->Update();
	if (changed && lastChanged < GetTicks())
	{
		changed = false;
		c->SetPage(std::max(pageTextbox->GetText().ToNumber<int>(true), 0));
	}
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
		String pageInfo = String::Build("of ", pageCount);
		pageCountLabel->SetText(pageInfo);
		int width = Graphics::textwidth(pageInfo);

		pageLabel->Position.X = WINDOWW/2-width-20;
		pageTextbox->Position.X = WINDOWW/2-width+11;
		pageTextbox->Size.X = width-4;
		//pageCountLabel->Position.X = WINDOWW/2+6;
		pageLabel->Visible = pageCountLabel->Visible = pageTextbox->Visible = true;

		pageInfo = String::Build(sender->GetPageNum());
		pageTextbox->SetText(pageInfo);
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

	std::vector<SaveFile*> saves = sender->GetSavesList();
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
		saveButton->SetActionCallback({
			[this, saveButton] {
				if (saveButton->GetSaveFile())
					c->OpenSave(saveButton->GetSaveFile());
			},
			nullptr,
			nullptr,
			[this, saveButton] {
				if (saveButton->GetSaveFile())
					c->Selected(saveButton->GetSaveFile()->GetDisplayName().ToUtf8(), saveButton->GetSelected());
			}
		});
		stampButtons.push_back(saveButton);
		AddComponent(saveButton);
		saveX++;
	}
}

void LocalBrowserView::NotifySelectedChanged(LocalBrowserModel * sender)
{
	std::vector<ByteString> selected = sender->GetSelected();
	for (size_t j = 0; j < stampButtons.size(); j++)
	{
		stampButtons[j]->SetSelected(false);
		for (size_t i = 0; i < selected.size(); i++)
		{
			if (stampButtons[j]->GetSaveFile()->GetDisplayName().ToUtf8() == selected[i])
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
	if (d)
		c->SetPageRelative(-d);
}

void LocalBrowserView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (key == SDLK_ESCAPE)
		c->Exit();
	else if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->SetMoveToFront(false);
}

void LocalBrowserView::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->SetMoveToFront(true);
}

LocalBrowserView::~LocalBrowserView() { }
