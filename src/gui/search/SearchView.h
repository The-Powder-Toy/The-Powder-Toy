#pragma once
#include "client/ClientListener.h"
#include "gui/interface/Window.h"
#include <vector>

namespace ui
{
	class RichLabel;
	class SaveButton;
	class Button;
	class Label;
	class Spinner;
	class Textbox;
	class DropDown;
}

class SearchModel;
class SearchController;

class SearchView: public ui::Window, public ClientListener
{
private:
	SearchController * c;
	std::vector<ui::SaveButton*> saveButtons;
	std::vector<ui::Button*> tagButtons;
	ui::Button * favButton;
	ui::Button * nextButton;
	ui::Button * previousButton;
	ui::Label * errorLabel;
	ui::Textbox * searchField;
	ui::Textbox * pageTextbox;
	ui::Label * pageLabel;
	ui::Label * pageCountLabel;
	ui::Label * tagsLabel;
	ui::RichLabel * motdLabel = nullptr;
	ui::DropDown * dateRange;
	ui::Button * sortButton;
	ui::Button * ownButton;
	ui::Spinner * loadingSpinner;

	ui::Button * removeSelected;
	ui::Button * unpublishSelected;
	ui::Button * favouriteSelected;
	ui::Button * clearSelection;
	void searchHelp();
	void clearSearch();
	void doSearch();
	void textChanged();
	bool changed;
	unsigned int lastChanged;
	int pageCount;
	bool publishButtonShown;
public:
	void NotifyTagListChanged(SearchModel * sender);
	void NotifySaveListChanged(SearchModel * sender);
	void NotifySelectedChanged(SearchModel * sender);
	void NotifyPageChanged(SearchModel * sender);
	void NotifyPeriodChanged(SearchModel * sender);
	void NotifySortChanged(SearchModel * sender);
	void NotifyShowOwnChanged(SearchModel * sender);
	void NotifyShowFavouriteChanged(SearchModel * sender);
	void NotifyAuthUserChanged(Client * sender) override;
	void NotifyMessageOfTheDay(Client * sender) override;
	void CheckAccess();
	void OnTryOkay(OkayMethod method) override;
    SearchView();
	virtual ~SearchView();
	void AttachController(SearchController * _c) { c = _c; }
	virtual void Search(const String&);
	void OnTick(float dt) override;
	void OnMouseWheel(int x, int y, int d) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;

};
