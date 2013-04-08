#include <sstream>

#include "SearchView.h"
#include "client/Client.h"
#include "gui/interface/Keys.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/Label.h"
#include "gui/interface/RichLabel.h"
#include "gui/interface/Textbox.h"
#include "Misc.h"

SearchView::SearchView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE)),
	saveButtons(vector<ui::SaveButton*>()),
	errorLabel(NULL),
	c(NULL)
{

	Client::Ref().AddListener(this);

	nextButton = new ui::Button(ui::Point(XRES+BARSIZE-52, YRES+MENUSIZE-18), ui::Point(50, 16), "Next \x95");
	previousButton = new ui::Button(ui::Point(1, YRES+MENUSIZE-18), ui::Point(50, 16), "\x96 Prev");
	infoLabel  = new ui::Label(ui::Point(260, YRES+MENUSIZE-18), ui::Point(XRES+BARSIZE-520, 16), "Page 1 of 1");
	tagsLabel  = new ui::Label(ui::Point(270, YRES+MENUSIZE-18), ui::Point(XRES+BARSIZE-540, 16), "\boPopular Tags:");
	motdLabel  = new ui::RichLabel(ui::Point(51, YRES+MENUSIZE-18), ui::Point(XRES+BARSIZE-102, 16), Client::Ref().GetMessageOfTheDay());

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
	searchField = new ui::Textbox(ui::Point(60, 10), ui::Point((XRES+BARSIZE)-238, 17), "", "[search]");
	searchField->Appearance.icon = IconSearch;
	searchField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	searchField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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
	sortButton = new ui::Button(ui::Point(XRES+BARSIZE-140, 10), ui::Point(61, 17), "Sort");
	sortButton->SetIcon(IconVoteSort);
	sortButton->SetTogglable(true);
	sortButton->SetActionCallback(new SortAction(this));
	sortButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	sortButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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
	ownButton = new ui::Button(ui::Point(XRES+BARSIZE-70, 10), ui::Point(61, 17), "My Own");
	ownButton->SetIcon(IconMyOwn);
	ownButton->SetTogglable(true);
	ownButton->SetActionCallback(new MyOwnAction(this));
	ownButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	ownButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(ownButton);

	class FavAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		FavAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->ShowFavourite(sender->GetToggleState());
		}
	};
	favButton = new ui::Button(searchField->Position+ui::Point(searchField->Size.X+15, 0), ui::Point(17, 17), "");
	favButton->SetIcon(IconFavourite);
	favButton->SetTogglable(true);
	favButton->Appearance.Margin.Left+=2;
	favButton->SetActionCallback(new FavAction(this));
	favButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	favButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	favButton->Appearance.BorderInactive = ui::Colour(170,170,170);
	AddComponent(favButton);
	
	class ClearSearchAction : public ui::ButtonAction
	{
		SearchView * v;
	public:
		ClearSearchAction(SearchView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->clearSearch();
		}
	};
	ui::Button * clearSearchButton = new ui::Button(searchField->Position+ui::Point(searchField->Size.X-1, 0), ui::Point(17, 17), "");
	clearSearchButton->SetIcon(IconClose);
	clearSearchButton->SetActionCallback(new ClearSearchAction(this));
	clearSearchButton->Appearance.Margin.Left+=2;
	clearSearchButton->Appearance.Margin.Top+=2;
	clearSearchButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	clearSearchButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	clearSearchButton->Appearance.BorderInactive = ui::Colour(170,170,170);
	AddComponent(clearSearchButton);

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
	nextButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	nextButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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
	previousButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previousButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(searchField);
	AddComponent(infoLabel);

	loadingSpinner = new ui::Spinner(ui::Point(((XRES+BARSIZE)/2)-12, ((YRES+MENUSIZE)/2)+12), ui::Point(24, 24));
	AddComponent(loadingSpinner);

	ui::Label * searchPrompt = new ui::Label(ui::Point(10, 10), ui::Point(50, 16), "Search:");
	searchPrompt->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	searchPrompt->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	CheckAccess();
}

void SearchView::NotifyMessageOfTheDay(Client * sender)
{
	motdLabel->SetText(sender->GetMessageOfTheDay());
}

void SearchView::doSearch()
{
	if (searchField->GetText().length() > 3 || !searchField->GetText().length())
		c->DoSearch(searchField->GetText());
}


void SearchView::clearSearch()
{
	searchField->SetText("");
	c->DoSearch(searchField->GetText(), true);
}

void SearchView::OnTryOkay(OkayMethod method)
{
	c->DoSearch(searchField->GetText(), true);
}

SearchView::~SearchView()
{
	Client::Ref().RemoveListener(this);
	RemoveComponent(nextButton);
	RemoveComponent(previousButton);
	RemoveComponent(infoLabel);

	for(int i = 0; i < saveButtons.size(); i++)
	{
		RemoveComponent(saveButtons[i]);
		delete saveButtons[i];
	}
	saveButtons.clear();

	delete nextButton;
	delete previousButton;
	delete infoLabel;
}

void SearchView::Search(std::string query)
{
	searchField->SetText(query);
	c->DoSearch(query, true);
}

void SearchView::NotifySortChanged(SearchModel * sender)
{
	if(sender->GetSort() == "best")
	{
		sortButton->SetToggleState(false);
		sortButton->SetText("By votes");
		sortButton->SetIcon(IconVoteSort);
	}
	else
	{
		sortButton->SetToggleState(true);
		sortButton->SetText("By date");
		sortButton->SetIcon(IconDateSort);
	}
}

void SearchView::NotifyShowOwnChanged(SearchModel * sender)
{
	ownButton->SetToggleState(sender->GetShowOwn());
	if(sender->GetShowOwn() || Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator)
	{
		unpublishSelected->Enabled = true;
		removeSelected->Enabled = true;
	}
	else if(sender->GetShowFavourite())
	{
		unpublishSelected->Enabled = false;
		removeSelected->Enabled = false;
	}
	else
	{
		unpublishSelected->Enabled = false;
		removeSelected->Enabled = false;
	}
}

void SearchView::NotifyShowFavouriteChanged(SearchModel * sender)
{
	favButton->SetToggleState(sender->GetShowFavourite());
	if(sender->GetShowFavourite())
	{
		unpublishSelected->Enabled = false;
		removeSelected->Enabled = false;
	}
	else if(sender->GetShowOwn() || Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator)
	{
		unpublishSelected->Enabled = true;
		removeSelected->Enabled = true;
	}
	else
	{
		unpublishSelected->Enabled = false;
		removeSelected->Enabled = false;
	}
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

void SearchView::NotifyAuthUserChanged(Client * sender)
{
	CheckAccess();
}

void SearchView::CheckAccess()
{
	if(c)
	{
		c->ClearSelection();

		if(ownButton->GetToggleState())
			ownButton->DoAction();
		if(favButton->GetToggleState())
			favButton->DoAction();
	}
	
	if(Client::Ref().GetAuthUser().ID)
	{
		ownButton->Enabled = true;
		favButton->Enabled = true;
		favouriteSelected->Enabled = true;

		if(Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator)
		{
			unpublishSelected->Enabled = true;
			removeSelected->Enabled = true;
			for(int i = 0; i < saveButtons.size(); i++)
			{
				saveButtons[i]->SetSelectable(true);
			}
		}

	}
	else
	{
		ownButton->Enabled = false;
		favButton->Enabled = false;


		favouriteSelected->Enabled = false;
		unpublishSelected->Enabled = false;
		removeSelected->Enabled = false;

		for(int i = 0; i < saveButtons.size(); i++)
		{
			saveButtons[i]->SetSelectable(false);
			saveButtons[i]->SetSelected(false);
		}
	}
}

void SearchView::NotifyTagListChanged(SearchModel * sender)
{
	int i = 0;
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 1;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	int tagWidth, tagHeight, tagX = 0, tagY = 0, tagsX = 6, tagsY = 4, tagPadding = 1;
	int tagAreaWidth, tagAreaHeight, tagXOffset, tagYOffset;

	vector<pair<string, int> > tags = sender->GetTagList();

	RemoveComponent(motdLabel);
	motdLabel->SetParentWindow(NULL);

	RemoveComponent(tagsLabel);
	tagsLabel->SetParentWindow(NULL);

	for(i = 0; i < tagButtons.size(); i++)
	{
		RemoveComponent(tagButtons[i]);
		delete tagButtons[i];
	}
	tagButtons.clear();

	buttonYOffset = 28;
	buttonXOffset = buttonPadding;
	buttonAreaWidth = Size.X;
	buttonAreaHeight = Size.Y - buttonYOffset - 18;

	if(sender->GetShowTags())
	{
		buttonYOffset += (buttonAreaHeight/savesY) - buttonPadding*2;
		buttonAreaHeight = Size.Y - buttonYOffset - 18;
		savesY--;

		tagXOffset = tagPadding;
		tagYOffset = 60;
		tagAreaWidth = Size.X;
		tagAreaHeight = ((buttonAreaHeight/savesY) - buttonPadding*2)-(tagYOffset-28)-5;
		tagWidth = (tagAreaWidth/tagsX) - tagPadding*2;
		tagHeight = (tagAreaHeight/tagsY) - tagPadding*2;

		AddComponent(tagsLabel);
		tagsLabel->Position.Y = tagYOffset-16;

		AddComponent(motdLabel);
		motdLabel->Position.Y = tagYOffset-30;
	}

	class TagAction: public ui::ButtonAction
	{
		SearchView * v;
		std::string tag;
	public:
		TagAction(SearchView * v, std::string tag) : v(v), tag(tag) {}
		virtual void ActionCallback(ui::Button * sender)
		{
			v->Search(tag);
		}
	};
	if(sender->GetShowTags())
	{
		for(i = 0; i < tags.size(); i++)
		{
			int maxTagVotes = tags[0].second;

			pair<string, int> tag = tags[i];
			
			if(tagX == tagsX)
			{
				if(tagY == tagsY-1)
					break;
				tagX = 0;
				tagY++;
			}

			int tagAlpha = 192;
			if (maxTagVotes)
				tagAlpha = 127+(128*tag.second)/maxTagVotes;

			ui::Button * tagButton;
			tagButton = new ui::Button(
				ui::Point(
						tagXOffset + tagPadding + tagX*(tagWidth+tagPadding*2),
						tagYOffset + tagPadding + tagY*(tagHeight+tagPadding*2)
					),
				ui::Point(tagWidth, tagHeight),
				tag.first
				);
			tagButton->SetActionCallback(new TagAction(this, tag.first));
			tagButton->Appearance.BorderInactive = ui::Colour(0, 0, 0);
			tagButton->Appearance.BorderHover = ui::Colour(0, 0, 0);
			tagButton->Appearance.BorderActive = ui::Colour(0, 0, 0);
			tagButton->Appearance.BackgroundHover = ui::Colour(0, 0, 0);

			tagButton->Appearance.TextInactive = ui::Colour(tagAlpha, tagAlpha, tagAlpha);
			tagButton->Appearance.TextHover = ui::Colour((tagAlpha*5)/6, (tagAlpha*5)/6, tagAlpha);
			AddComponent(tagButton);
			tagButtons.push_back(tagButton);
			tagX++;

		}
	}
}

void SearchView::NotifySaveListChanged(SearchModel * sender)
{
	int i = 0;
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 1;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	int tagWidth, tagHeight, tagX = 0, tagY = 0, tagsX = 6, tagsY = 4, tagPadding = 1;
	int tagAreaWidth, tagAreaHeight, tagXOffset, tagYOffset;

	vector<SaveInfo*> saves = sender->GetSaveList();
	//string messageOfTheDay = sender->GetMessageOfTheDay();

	if(sender->GetShowFavourite())
		favouriteSelected->SetText("Unfavourite");
	else
		favouriteSelected->SetText("Favourite");

	Client::Ref().ClearThumbnailRequests();
	for(i = 0; i < saveButtons.size(); i++)
	{
		RemoveComponent(saveButtons[i]);
	}
	if(!sender->GetSavesLoaded())
	{
		nextButton->Enabled = false;
		previousButton->Enabled = false;
		favButton->Enabled = false;
	}
	else
	{
		nextButton->Enabled = true;
		previousButton->Enabled = true;
		if (Client::Ref().GetAuthUser().ID)
			favButton->Enabled = true;
	}
	if (!sender->GetSavesLoaded() || favButton->GetToggleState())
	{
		ownButton->Enabled = false;
		sortButton->Enabled = false;
	}
	else
	{
		if (Client::Ref().GetAuthUser().ID)
			ownButton->Enabled = true;
		sortButton->Enabled = true;
	}
	if(!saves.size())
	{
		loadingSpinner->Visible = false;
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
		for(i = 0; i < saveButtons.size(); i++)
		{
			delete saveButtons[i];
		}
		saveButtons.clear();

		buttonYOffset = 28;
		buttonXOffset = buttonPadding;
		buttonAreaWidth = Size.X;
		buttonAreaHeight = Size.Y - buttonYOffset - 18;

		if(sender->GetShowTags())
		{
			buttonYOffset += (buttonAreaHeight/savesY) - buttonPadding*2;
			buttonAreaHeight = Size.Y - buttonYOffset - 18;
			savesY--;

			tagXOffset = tagPadding;
			tagYOffset = 60;
			tagAreaWidth = Size.X;
			tagAreaHeight = ((buttonAreaHeight/savesY) - buttonPadding*2)-(tagYOffset-28)-5;
			tagWidth = (tagAreaWidth/tagsX) - tagPadding*2;
			tagHeight = (tagAreaHeight/tagsY) - tagPadding*2;
		}

		buttonWidth = (buttonAreaWidth/savesX) - buttonPadding*2;
		buttonHeight = (buttonAreaHeight/savesY) - buttonPadding*2;



		class SaveOpenAction: public ui::SaveButtonAction
		{
			SearchView * v;
		public:
			SaveOpenAction(SearchView * _v) { v = _v; }
			virtual void ActionCallback(ui::SaveButton * sender)
			{
				v->c->OpenSave(sender->GetSave()->GetID(), sender->GetSave()->GetVersion());
			}
			virtual void SelectedCallback(ui::SaveButton * sender)
			{
				v->c->Selected(sender->GetSave()->GetID(), sender->GetSelected());
			}
			virtual void AltActionCallback(ui::SaveButton * sender)
			{
				stringstream search;
				search << "history:" << sender->GetSave()->GetID();
				v->Search(search.str());
			}
			virtual void AltActionCallback2(ui::SaveButton * sender)
			{
				v->Search("user:"+sender->GetSave()->GetUserName());
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
			saveButton->AddContextMenu(0);
			saveButton->SetActionCallback(new SaveOpenAction(this));
			if(Client::Ref().GetAuthUser().ID)
				saveButton->SetSelectable(true);
			if (saves[i]->GetUserName() == Client::Ref().GetAuthUser().Username || Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator)
				saveButton->SetShowVotes(true);
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

