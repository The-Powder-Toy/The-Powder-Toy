#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <vector>
#include "SearchController.h"
#include "client/ClientListener.h"

using namespace std;

namespace ui
{
	class RichLabel;
	class SaveButton;
	class Button;
	class Label;
	class Spinner;
	class Textbox;
}

class SearchModel;
class SearchController;

class SearchView: public ui::Window, public ClientListener
{
private:
	SearchController * c;
	vector<ui::SaveButton*> saveButtons;
	vector<ui::Button*> tagButtons;
	ui::Button * favButton;
	ui::Button * nextButton;
	ui::Button * previousButton;
	ui::Label * errorLabel;
	ui::Textbox * searchField;
	ui::Textbox * pageTextbox;
	ui::Label * pageLabel;
	ui::Label * pageCountLabel;
	ui::Label * tagsLabel;
	ui::RichLabel * motdLabel;
	ui::Button * sortButton;
	ui::Button * ownButton;
	ui::Spinner * loadingSpinner;

	ui::Button * removeSelected;
	ui::Button * unpublishSelected;
	ui::Button * favouriteSelected;
	ui::Button * clearSelection;
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
	void NotifySortChanged(SearchModel * sender);
	void NotifyShowOwnChanged(SearchModel * sender);
	void NotifyShowFavouriteChanged(SearchModel * sender);
	void NotifyAuthUserChanged(Client * sender);
	void NotifyMessageOfTheDay(Client * sender);
	void CheckAccess();
	virtual void OnTryOkay(OkayMethod method);
    SearchView();
	virtual ~SearchView();
	void AttachController(SearchController * _c) { c = _c; }
	virtual void Search(std::string);
	virtual void OnTick(float dt);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);

};

#endif // SEARCHVIEW_H
