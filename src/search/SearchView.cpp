#include "SearchView.h"
#include "interface/SaveButton.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "Misc.h"

SearchView::SearchView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE)),
	saveButtons(vector<ui::SaveButton*>()),
	errorLabel(NULL)
{
	nextButton = new ui::Button(ui::Point(XRES+BARSIZE-52, YRES+MENUSIZE-18), ui::Point(50, 16), "Next \x95");
	previousButton = new ui::Button(ui::Point(1, YRES+MENUSIZE-18), ui::Point(50, 16), "\x96 Prev");

	class SearchAction : public ui::TextboxAction
	{
		SearchView * v;
	public:
		SearchAction(SearchView * _v) { v = _v; }
		void TextChangedCallback(ui::Textbox * sender)
		{
			v->doSearch();
		}
	};
	searchField = new ui::Textbox(ui::Point(60, 10), ui::Point((XRES+BARSIZE)-((50*2)+16+10+50+10), 16), "");
	searchField->SetAlignment(AlignLeft, AlignBottom);
	searchField->SetActionCallback(new SearchAction(this));

	nextButton->SetAlignment(AlignRight, AlignBottom);
	previousButton->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(searchField);

	ui::Label * searchPrompt = new ui::Label(ui::Point(10, 10), ui::Point(50, 16), "Search:");
	searchPrompt->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(searchPrompt);
}

void SearchView::doSearch()
{
	c->DoSearch(searchField->GetText());
}

SearchView::~SearchView()
{
}

void SearchView::NotifySaveListChanged(SearchModel * sender)
{
	int i = 0;
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 4, savesY = 3, buttonPadding = 2;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<Save*> saves = sender->GetSaveList();
	if(!saves.size())
	{
		if(!errorLabel)
		{
			errorLabel = new ui::Label(ui::Point(((XRES+BARSIZE)/2)-100, ((YRES+MENUSIZE)/2)-6), ui::Point(200, 12), "Error");
			AddComponent(errorLabel);
		}
		if(sender->GetLastError().length())
			errorLabel->SetText(sender->GetLastError());
		else
			errorLabel->SetText("No saves found");
	}
	else
	{
		if(errorLabel)
		{
			RemoveComponent(errorLabel);
			delete errorLabel;
			errorLabel = NULL;
		}
		buttonXOffset = 0;
		buttonYOffset = 50;
		buttonAreaWidth = Size.X;
		buttonAreaHeight = Size.Y - buttonYOffset - 18;
		buttonWidth = (buttonAreaWidth/savesX) - buttonPadding*2;
		buttonHeight = (buttonAreaHeight/savesY) - buttonPadding*2;
		for(i = 0; i < saveButtons.size(); i++)
		{
			RemoveComponent(saveButtons[i]);
			
			delete saveButtons[i];
		}
		saveButtons.clear();
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
			saveButtons.push_back(saveButton);
			AddComponent(saveButton);
			saveX++;
		}
	}
}
