#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <vector>
#include "SearchController.h"
#include "interface/SaveButton.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/Spinner.h"
#include "interface/Textbox.h"

using namespace std;

class SearchModel;
class SearchController;

class SearchView: public ui::Window
{
private:
	SearchController * c;
	vector<ui::SaveButton*> saveButtons;
	ui::Button * nextButton;
	ui::Button * previousButton;
	ui::Label * errorLabel;
	ui::Textbox * searchField;
	ui::Label * infoLabel;
	ui::Button * sortButton;
	ui::Button * ownButton;
	ui::Spinner * loadingSpinner;

	ui::Button * removeSelected;
	ui::Button * unpublishSelected;
	ui::Button * favouriteSelected;
	ui::Button * clearSelection;
	void doSearch();
public:
	void NotifySaveListChanged(SearchModel * sender);
	void NotifySelectedChanged(SearchModel * sender);
	void NotifyPageChanged(SearchModel * sender);
	void NotifySortChanged(SearchModel * sender);
	void NotifyShowOwnChanged(SearchModel * sender);
    SearchView();
	virtual ~SearchView();
	void AttachController(SearchController * _c) { c = _c; }
	virtual void OnTick(float dt);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);

};

#endif // SEARCHVIEW_H
