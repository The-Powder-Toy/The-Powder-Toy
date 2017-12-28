#include <sstream>

#include "SearchView.h"
#include "client/Client.h"
#include "gui/interface/Keys.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/RichLabel.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Spinner.h"
#include "Misc.h"
#include "Format.h"
#include "PowderToy.h"

SearchView::SearchView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	c(NULL),
	saveButtons(vector<ui::SaveButton*>()),
	errorLabel(NULL),
	changed(true),
	lastChanged(0),
	pageCount(0),
	publishButtonShown(false)
{

	Client::Ref().AddListener(this);

	nextButton = new ui::Button(ui::Point(WINDOWW-52, WINDOWH-18), ui::Point(50, 16), "Next \x95");
	previousButton = new ui::Button(ui::Point(2, WINDOWH-18), ui::Point(50, 16), "\x96 Prev");
	tagsLabel  = new ui::Label(ui::Point(270, WINDOWH-18), ui::Point(WINDOWW-540, 16), "\boPopular Tags:");
	motdLabel  = new ui::RichLabel(ui::Point(51, WINDOWH-18), ui::Point(WINDOWW-102, 16), Client::Ref().GetMessageOfTheDay());

	class PageNumAction : public ui::TextboxAction
	{
		SearchView * v;
	public:
		PageNumAction(SearchView * _v) { v = _v; }
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
	searchField = new ui::Textbox(ui::Point(60, 10), ui::Point(WINDOWW-238, 17), "", "[search]");
	searchField->Appearance.icon = IconSearch;
	searchField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	searchField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	searchField->SetActionCallback(new SearchAction(this));
	FocusComponent(searchField);


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
	sortButton = new ui::Button(ui::Point(WINDOWW-140, 10), ui::Point(61, 17), "Sort");
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
	ownButton = new ui::Button(ui::Point(WINDOWW-70, 10), ui::Point(61, 17), "My Own");
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
	nextButton->Visible = false;
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
	previousButton->Visible = false;
	AddComponent(nextButton);
	AddComponent(previousButton);
	AddComponent(searchField);

	loadingSpinner = new ui::Spinner(ui::Point((WINDOWW/2)-12, (WINDOWH/2)+12), ui::Point(24, 24));
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
			v->c->UnpublishSelected(v->publishButtonShown);
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

	removeSelected = new ui::Button(ui::Point(((WINDOWW-415)/2), WINDOWH-18), ui::Point(100, 16), "Delete");
	removeSelected->Visible = false;
	removeSelected->SetActionCallback(new RemoveSelectedAction(this));
	AddComponent(removeSelected);

	unpublishSelected = new ui::Button(ui::Point(((WINDOWW-415)/2)+105, WINDOWH-18), ui::Point(100, 16), "Unpublish");
	unpublishSelected->Visible = false;
	unpublishSelected->SetActionCallback(new UnpublishSelectedAction(this));
	AddComponent(unpublishSelected);

	favouriteSelected = new ui::Button(ui::Point(((WINDOWW-415)/2)+210, WINDOWH-18), ui::Point(100, 16), "Favourite");
	favouriteSelected->Visible = false;
	favouriteSelected->SetActionCallback(new FavouriteSelectedAction(this));
	AddComponent(favouriteSelected);

	clearSelection = new ui::Button(ui::Point(((WINDOWW-415)/2)+315, WINDOWH-18), ui::Point(100, 16), "Clear selection");
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

void SearchView::textChanged()
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

void SearchView::OnTryOkay(OkayMethod method)
{
	c->DoSearch(searchField->GetText(), true);
}

SearchView::~SearchView()
{
	Client::Ref().RemoveListener(this);
	RemoveComponent(nextButton);
	RemoveComponent(previousButton);
	RemoveComponent(pageTextbox);
	RemoveComponent(pageLabel);
	RemoveComponent(pageCountLabel);
	delete nextButton;
	delete previousButton;
	delete pageTextbox;
	delete pageLabel;
	delete pageCountLabel;

	for (size_t i = 0; i < saveButtons.size(); i++)
	{
		RemoveComponent(saveButtons[i]);
		delete saveButtons[i];
	}
	saveButtons.clear();
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
	pageCount = sender->GetPageCount();
	if (!sender->GetSaveList().size()) //no saves
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
	if(sender->GetPageNum() >= sender->GetPageCount())
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
	if (c)
	{
		c->ClearSelection();

		if(ownButton->GetToggleState())
			ownButton->DoAction();
		if(favButton->GetToggleState())
			favButton->DoAction();
	}
	
	if (Client::Ref().GetAuthUser().UserID)
	{
		ownButton->Enabled = true;
		favButton->Enabled = true;
		favouriteSelected->Enabled = true;

		if (Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator)
		{
			unpublishSelected->Enabled = true;
			removeSelected->Enabled = true;
			for (size_t i = 0; i < saveButtons.size(); i++)
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

		for (size_t i = 0; i < saveButtons.size(); i++)
		{
			saveButtons[i]->SetSelectable(false);
			saveButtons[i]->SetSelected(false);
		}
	}
}

void SearchView::NotifyTagListChanged(SearchModel * sender)
{
	int savesY = 4, buttonPadding = 1;
	int buttonAreaHeight, buttonYOffset;

	int tagWidth = 0, tagHeight = 0, tagX = 0, tagY = 0, tagsX = 6, tagsY = 4, tagPadding = 1;
	int tagAreaWidth, tagAreaHeight, tagXOffset = 0, tagYOffset = 0;

	vector<pair<string, int> > tags = sender->GetTagList();

	RemoveComponent(motdLabel);
	motdLabel->SetParentWindow(NULL);

	RemoveComponent(tagsLabel);
	tagsLabel->SetParentWindow(NULL);

	for (size_t i = 0; i < tagButtons.size(); i++)
	{
		RemoveComponent(tagButtons[i]);
		delete tagButtons[i];
	}
	tagButtons.clear();

	buttonYOffset = 28;
	buttonAreaHeight = Size.Y - buttonYOffset - 18;

	if (sender->GetShowTags())
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
	if (sender->GetShowTags())
	{
		for (size_t i = 0; i < tags.size(); i++)
		{
			int maxTagVotes = tags[0].second;

			pair<string, int> tag = tags[i];
			
			if (tagX == tagsX)
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
	int buttonWidth, buttonHeight, saveX = 0, saveY = 0, savesX = 5, savesY = 4, buttonPadding = 1;
	int buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;

	vector<SaveInfo*> saves = sender->GetSaveList();
	//string messageOfTheDay = sender->GetMessageOfTheDay();

	if(sender->GetShowFavourite())
		favouriteSelected->SetText("Unfavourite");
	else
		favouriteSelected->SetText("Favourite");

	Client::Ref().ClearThumbnailRequests();
	for (size_t i = 0; i < saveButtons.size(); i++)
	{
		RemoveComponent(saveButtons[i]);
	}
	if (!sender->GetSavesLoaded())
	{
		nextButton->Enabled = false;
		previousButton->Enabled = false;
		favButton->Enabled = false;
	}
	else
	{
		nextButton->Enabled = true;
		previousButton->Enabled = true;
		if (Client::Ref().GetAuthUser().UserID)
			favButton->Enabled = true;
	}
	if (!sender->GetSavesLoaded() || favButton->GetToggleState())
	{
		ownButton->Enabled = false;
		sortButton->Enabled = false;
	}
	else
	{
		if (Client::Ref().GetAuthUser().UserID)
			ownButton->Enabled = true;
		sortButton->Enabled = true;
	}
	if (!saves.size())
	{
		loadingSpinner->Visible = false;
		if (!errorLabel)
		{
			errorLabel = new ui::Label(ui::Point((WINDOWW/2)-100, (WINDOWH/2)-6), ui::Point(200, 12), "Error");
			AddComponent(errorLabel);
		}
		if (!sender->GetSavesLoaded())
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
		if (errorLabel)
		{
			RemoveComponent(errorLabel);
			delete errorLabel;
			errorLabel = NULL;
		}
		for (size_t i = 0; i < saveButtons.size(); i++)
		{
			delete saveButtons[i];
		}
		saveButtons.clear();

		buttonYOffset = 28;
		buttonXOffset = buttonPadding;
		buttonAreaWidth = Size.X;
		buttonAreaHeight = Size.Y - buttonYOffset - 18;

		if (sender->GetShowTags())
		{
			buttonYOffset += (buttonAreaHeight/savesY) - buttonPadding*2;
			buttonAreaHeight = Size.Y - buttonYOffset - 18;
			savesY--;
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
		for (size_t i = 0; i < saves.size(); i++)
		{
			if (saveX == savesX)
			{
				if (saveY == savesY-1)
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
			if(Client::Ref().GetAuthUser().UserID)
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
	size_t published = 0;
	for (size_t j = 0; j < saveButtons.size(); j++)
	{
		saveButtons[j]->SetSelected(false);
		for (size_t i = 0; i < selected.size(); i++)
		{
			if (saveButtons[j]->GetSave()->GetID() == selected[i])
			{
				saveButtons[j]->SetSelected(true);
				if (saveButtons[j]->GetSave()->GetPublished())
					published++;
			}
		}
	}

	if (selected.size())
	{
		removeSelected->Visible = true;
		unpublishSelected->Visible = true;
		favouriteSelected->Visible = true;
		clearSelection->Visible = true;
		pageTextbox->Visible = false;
		pageLabel->Visible = false;
		pageCountLabel->Visible = false;
		if (published <= selected.size()/2)
		{
			unpublishSelected->SetText("Publish");
			publishButtonShown = true;
		}
		else
		{
			unpublishSelected->SetText("Unpublish");
			publishButtonShown = false;
		}
	}
	else if (removeSelected->Visible)
	{
		removeSelected->Visible = false;
		unpublishSelected->Visible = false;
		favouriteSelected->Visible = false;
		clearSelection->Visible = false;
		pageTextbox->Visible = true;
		pageLabel->Visible = true;
		pageCountLabel->Visible = true;
	}
}

void SearchView::OnTick(float dt)
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
	if (key == SDLK_ESCAPE)
		c->Exit();
	else if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->InstantOpen(true);
}

void SearchView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->InstantOpen(false);
}
