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
	searchField = new ui::Textbox(ui::Point(60, 10), ui::Point((XRES+BARSIZE)-((60*2)+16+10+50+10), 16), "");
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
	sortButton = new ui::Button(ui::Point(XRES+BARSIZE-60-60-16-10+5, 10), ui::Point(60, 16), "Sort");
	sortButton->SetActionCallback(new SortAction(this));
	sortButton->SetAlignment(AlignCentre, AlignBottom);
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
	ownButton = new ui::Button(ui::Point(XRES+BARSIZE-60-16-10+10, 10), ui::Point(60, 16), "My Own");
	ownButton->SetTogglable(true);
	ownButton->SetActionCallback(new MyOwnAction(this));
	if(!Client::Ref().GetAuthUser().ID)
		ownButton->Enabled = false;
	ownButton->SetAlignment(AlignCentre, AlignBottom);
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

	loadingSpinner = new ui::Spinner(ui::Point(((XRES+BARSIZE)/2)-12, ((YRES+MENUSIZE)/2)+12), ui::Point(24, 24));
	AddComponent(loadingSpinner);

	ui::Label * searchPrompt = new ui::Label(ui::Point(10, 10), ui::Point(50, 16), "Search:");
	searchPrompt->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(searchPrompt);

	class RemoveSelectedAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		RemoveSelectedAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->RemoveSelected();
		}
	};

	class UnpublishSelectedAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		UnpublishSelectedAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->UnpublishSelected();
		}
	};

	class FavouriteSelectedAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		FavouriteSelectedAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->FavouriteSelected();
		}
	};

	class ClearSelectionAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		ClearSelectionAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->ClearSelection();
		}
	};

	removeSelected = new ui::Button(ui::Point((((XRES+BARSIZE)-415)/2), YRES+MENUSIZE-18), ui::Point(100, 16), "Delete");
	removeSelected->Visible = false;
	removeSelected->SetActionCallback(new RemoveSelectedAction(this));
	AddComponent(removeSelected);

	unpublishSelected = new ui::Button(ui::Point((((XRES+BARSIZE)-415)/2)+105, YRES+MENUSIZE-18), ui::Point(100, 16), "Unpublish");
	unpublishSelected->Visible = false;
	unpublishSelected->SetActionCallback(new UnpublishSelectedAction(this));
	AddComponent(unpublishSelected);

	favouriteSelected = new ui::Button(ui::Point((((XRES+BARSIZE)-415)/2)+210, YRES+MENUSIZE-18), ui::Point(100, 16), "Favourite");
	favouriteSelected->Visible = false;
	favouriteSelected->SetActionCallback(new FavouriteSelectedAction(this));
	AddComponent(favouriteSelected);

	clearSelection = new ui::Button(ui::Point((((XRES+BARSIZE)-415)/2)+315, YRES+MENUSIZE-18), ui::Point(100, 16), "Clear selection");
	clearSelection->Visible = false;
	clearSelection->SetActionCallback(new ClearSelectionAction(this));
	AddComponent(clearSelection);
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
    sortButton->SetText("Show "+sender->GetSort());
}

void SearchView::NotifyShowOwnChanged(SearchModel * sender)
{
    ownButton->SetToggleState(sender->GetShowOwn());
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
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 2;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<Save*> saves = sender->GetSaveList();
	Client::Ref().ClearThumbnailRequests();
	for(i = 0; i < saveButtons.size(); i++)
	{
		RemoveComponent(saveButtons[i]);
		delete saveButtons[i];
	}
	saveButtons.clear();
	if(!sender->GetSavesLoaded())
	{
		nextButton->Enabled = false;
		previousButton->Enabled = false;
	}
	else
	{
		nextButton->Enabled = true;
		previousButton->Enabled = true;
	}
	if(!saves.size())
	{
		if(!errorLabel)
		{
			errorLabel = new ui::Label(ui::Point(((XRES+BARSIZE)/2)-100, ((YRES+MENUSIZE)/2)-6), ui::Point(200, 12), "Error");
			AddComponent(errorLabel);
		}
		if(!sender->GetSavesLoaded())
		{
			errorLabel->SetText("Loading...");
			loadingSpinner->Visible = true;
		}
		else
		{
			if(sender->GetLastError().length())
				errorLabel->SetText("\bo" + sender->GetLastError());
			else
				errorLabel->SetText("\boNo saves found");
		}
	}
	else
	{
		loadingSpinner->Visible = false;
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
		class SaveOpenAction: public ui::SaveButtonAction
		{
			SearchView * v;
		public:
			SaveOpenAction(SearchView * _v) { v = _v; }
			virtual void ActionCallback(ui::SaveButton * sender)
			{
				v->c->OpenSave(sender->GetSave()->GetID());
			}
			virtual void SelectedCallback(ui::SaveButton * sender)
			{
				v->c->Selected(sender->GetSave()->GetID(), sender->GetSelected());
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
			if(Client::Ref().GetAuthUser().ID)
				saveButton->SetSelectable(true);
			saveButtons.push_back(saveButton);
			AddComponent(saveButton);
			saveX++;
		}
	}
}

void SearchView::NotifySelectedChanged(SearchModel * sender)
{
	vector<int> selected = sender->GetSelected();
	for(int j = 0; j < saveButtons.size(); j++)
	{
		saveButtons[j]->SetSelected(false);
		for(int i = 0; i < selected.size(); i++)
		{
			if(saveButtons[j]->GetSave()->GetID()==selected[i])
				saveButtons[j]->SetSelected(true);
		}
	}

	if(selected.size())
	{
		removeSelected->Visible = true;
		unpublishSelected->Visible = true;
		favouriteSelected->Visible = true;
		clearSelection->Visible = true;
	}
	else
	{
		removeSelected->Visible = false;
		unpublishSelected->Visible = false;
		favouriteSelected->Visible = false;
		clearSelection->Visible = false;
	}
}

void SearchView::OnTick(float dt)
{
	c->Update();
}

void SearchView::OnMouseWheel(int x, int y, int d)
{
	if(!d)
		return;
	if(d<0)
		c->NextPage();
	else
		c->PrevPage();
}
void SearchView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(key==KEY_ESCAPE)
		c->Exit();
}

