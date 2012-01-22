#include <sstream>

#include "SearchView.h"
#include "client/Client.h"
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
	infoLabel  = new ui::Label(ui::Point(51, YRES+MENUSIZE-18), ui::Point(XRES+BARSIZE-102, 16), "Loading...");

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

	class SortAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		SortAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->ChangeSort();
		}
	};
	sortButton = new ui::Button(ui::Point(XRES+BARSIZE-50-50-16-10, 10), ui::Point(50, 16), "Sort");
	sortButton->SetActionCallback(new SortAction(this));
	sortButton->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(sortButton);

	class MyOwnAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		MyOwnAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->ShowOwn(sender->GetToggleState());
		}
	};
	ownButton = new ui::Button(ui::Point(XRES+BARSIZE-50-16-10, 10), ui::Point(50, 16), "My Own");
	ownButton->SetTogglable(true);
	ownButton->SetActionCallback(new MyOwnAction(this));
	ownButton->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(ownButton);

	class NextPageAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		NextPageAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->NextPage();
		}
	};
	nextButton->SetActionCallback(new NextPageAction(this));
	nextButton->SetAlignment(AlignRight, AlignBottom);
	class PrevPageAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		PrevPageAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->PrevPage();
		}
	};
	previousButton->SetActionCallback(new PrevPageAction(this));
	previousButton->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(searchField);
	AddComponent(infoLabel);

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

void SearchView::NotifySortChanged(SearchModel * sender)
{
    sortButton->SetText("Sort: "+sender->GetSort());
}

void SearchView::NotifyShowOwnChanged(SearchModel * sender)
{
    sortButton->SetToggleState(sender->GetShowOwn());
}

void SearchView::NotifyPageChanged(SearchModel * sender)
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

void SearchView::NotifySaveListChanged(SearchModel * sender)
{
	int i = 0;
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 4, savesY = 3, buttonPadding = 2;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<Save*> saves = sender->GetSaveList();
	Client::Ref().ClearThumbnailRequests();
	for(i = 0; i < saveButtons.size(); i++)
	{
		RemoveComponent(saveButtons[i]);
		delete saveButtons[i];
	}
	saveButtons.clear();
	if(!saves.size())
	{
		if(!errorLabel)
		{
			errorLabel = new ui::Label(ui::Point(((XRES+BARSIZE)/2)-100, ((YRES+MENUSIZE)/2)-6), ui::Point(200, 12), "Error");
			AddComponent(errorLabel);
		}
		if(sender->GetLastError().length())
			errorLabel->SetText("\bo" + sender->GetLastError());
		else
			errorLabel->SetText("\boNo saves found");
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
