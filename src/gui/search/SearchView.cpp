#include "SearchView.h"

#include "SearchController.h"
#include "SearchModel.h"

#include "client/Client.h"
#include "client/SaveInfo.h"

#include "gui/interface/Keys.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/RichLabel.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Spinner.h"

#include "PowderToy.h"
#include "Config.h"

#include "graphics/Graphics.h"

#ifdef GetUserName
# undef GetUserName // dammit windows
#endif

SearchView::SearchView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	c(NULL),
	saveButtons(std::vector<ui::SaveButton*>()),
	errorLabel(NULL),
	changed(true),
	lastChanged(0),
	pageCount(0),
	publishButtonShown(false)
{

	Client::Ref().AddListener(this);

	nextButton = new ui::Button(ui::Point(WINDOWW-52, WINDOWH-18), ui::Point(50, 16), String("Next ") + 0xE015);
	previousButton = new ui::Button(ui::Point(2, WINDOWH-18), ui::Point(50, 16), 0xE016 + String(" Prev"));
	tagsLabel  = new ui::Label(ui::Point(270, WINDOWH-18), ui::Point(WINDOWW-540, 16), "\boPopular Tags:");
	try
	{
		motdLabel  = new ui::RichLabel(ui::Point(51, WINDOWH-18), ui::Point(WINDOWW-102, 16), Client::Ref().GetMessageOfTheDay());
	}
	catch (std::exception & e) { }

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

	searchField = new ui::Textbox(ui::Point(60, 10), ui::Point(WINDOWW-238, 17), "", "[search]");
	searchField->Appearance.icon = IconSearch;
	searchField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	searchField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	searchField->SetActionCallback({ [this] { doSearch(); } });
	FocusComponent(searchField);

	sortButton = new ui::Button(ui::Point(WINDOWW-140, 10), ui::Point(61, 17), "Sort");
	sortButton->SetIcon(IconVoteSort);
	sortButton->SetTogglable(true);
	sortButton->SetActionCallback({ [this] { c->ChangeSort(); } });
	sortButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	sortButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(sortButton);

	ownButton = new ui::Button(ui::Point(WINDOWW-70, 10), ui::Point(61, 17), "My Own");
	ownButton->SetIcon(IconMyOwn);
	ownButton->SetTogglable(true);
	ownButton->SetActionCallback({ [this] { c->ShowOwn(ownButton->GetToggleState()); } });
	ownButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	ownButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(ownButton);

	favButton = new ui::Button(searchField->Position+ui::Point(searchField->Size.X+15, 0), ui::Point(17, 17), "");
	favButton->SetIcon(IconFavourite);
	favButton->SetTogglable(true);
	favButton->Appearance.Margin.Left+=2;
	favButton->SetActionCallback({ [this] { c->ShowFavourite(favButton->GetToggleState()); } });
	favButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	favButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	favButton->Appearance.BorderInactive = ui::Colour(170,170,170);
	AddComponent(favButton);

	ui::Button * clearSearchButton = new ui::Button(searchField->Position+ui::Point(searchField->Size.X-1, 0), ui::Point(17, 17), "");
	clearSearchButton->SetIcon(IconClose);
	clearSearchButton->SetActionCallback({ [this] { clearSearch(); } });
	clearSearchButton->Appearance.Margin.Left+=2;
	clearSearchButton->Appearance.Margin.Top+=2;
	clearSearchButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	clearSearchButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	clearSearchButton->Appearance.BorderInactive = ui::Colour(170,170,170);
	AddComponent(clearSearchButton);

	nextButton->SetActionCallback({ [this] { c->SetPageRelative(1); } });
	nextButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	nextButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	nextButton->Visible = false;
	previousButton->SetActionCallback({ [this] { c->SetPageRelative(-1); } });
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

	removeSelected = new ui::Button(ui::Point(((WINDOWW-415)/2), WINDOWH-18), ui::Point(100, 16), "Delete");
	removeSelected->Visible = false;
	removeSelected->SetActionCallback({ [this] { c->RemoveSelected(); } });
	AddComponent(removeSelected);

	unpublishSelected = new ui::Button(ui::Point(((WINDOWW-415)/2)+105, WINDOWH-18), ui::Point(100, 16), "Unpublish");
	unpublishSelected->Visible = false;
	unpublishSelected->SetActionCallback({ [this] { c->UnpublishSelected(publishButtonShown); } });
	AddComponent(unpublishSelected);

	favouriteSelected = new ui::Button(ui::Point(((WINDOWW-415)/2)+210, WINDOWH-18), ui::Point(100, 16), "Favourite");
	favouriteSelected->Visible = false;
	favouriteSelected->SetActionCallback({ [this] { c->FavouriteSelected(); } });
	AddComponent(favouriteSelected);

	clearSelection = new ui::Button(ui::Point(((WINDOWW-415)/2)+315, WINDOWH-18), ui::Point(100, 16), "Clear selection");
	clearSelection->Visible = false;
	clearSelection->SetActionCallback({ [this] { c->ClearSelection(); } });
	AddComponent(clearSelection);

	CheckAccess();
}

void SearchView::NotifyMessageOfTheDay(Client * sender)
{
	if (motdLabel)
	{
		try
		{
			motdLabel->SetText(sender->GetMessageOfTheDay());
		}
		catch (std::exception & e)
		{
			motdLabel = nullptr;
		}
	}

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
	int num = pageTextbox->GetText().ToNumber<int>(true);
	if (num < 0) //0 is allowed so that you can backspace the 1
		pageTextbox->SetText("1");
	else if (num > pageCount)
		pageTextbox->SetText(String::Build(pageCount));
	changed = true;
	lastChanged = GetTicks()+600;
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

void SearchView::Search(String query)
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

	std::vector<std::pair<ByteString, int> > tags = sender->GetTagList();

	if (motdLabel)
	{
		RemoveComponent(motdLabel);
		motdLabel->SetParentWindow(NULL);
	}

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

		if (motdLabel)
		{
			AddComponent(motdLabel);
			motdLabel->Position.Y = tagYOffset-30;
		}
	}

	if (sender->GetShowTags())
	{
		for (size_t i = 0; i < tags.size(); i++)
		{
			int maxTagVotes = tags[0].second;

			std::pair<ByteString, int> tag = tags[i];

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
				tag.first.FromUtf8()
				);
			tagButton->SetActionCallback({ [this, tag] {
				Search(tag.first.FromUtf8());
			} });
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

	std::vector<SaveInfo*> saves = sender->GetSaveList();
	//string messageOfTheDay = sender->GetMessageOfTheDay();

	if(sender->GetShowFavourite())
		favouriteSelected->SetText("Unfavourite");
	else
		favouriteSelected->SetText("Favourite");

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
			saveButton->SetActionCallback({
				[this, saveButton] { c->OpenSave(saveButton->GetSave()->GetID(), saveButton->GetSave()->GetVersion()); },
				[this, saveButton] { Search(String::Build("history:", saveButton->GetSave()->GetID())); },
				[this, saveButton] { Search(String::Build("user:", saveButton->GetSave()->GetUserName().FromUtf8())); },
				[this, saveButton] { c->Selected(saveButton->GetSave()->GetID(), saveButton->GetSelected()); }
			});
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
	std::vector<int> selected = sender->GetSelected();
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
	if (changed && lastChanged < GetTicks())
	{
		changed = false;
		c->SetPage(std::max(pageTextbox->GetText().ToNumber<int>(true), 0));
	}
}

void SearchView::OnMouseWheel(int x, int y, int d)
{
	if (d)
		c->SetPageRelative(-d);
}
void SearchView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (key == SDLK_ESCAPE)
		c->Exit();
	else if ((focusedComponent_ != pageTextbox && focusedComponent_ != searchField) && scan == SDL_SCANCODE_A && ctrl)
		c->SelectAllSaves();
	else if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->InstantOpen(true);
}

void SearchView::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (key == SDLK_LCTRL || key == SDLK_RCTRL)
		c->InstantOpen(false);
}
