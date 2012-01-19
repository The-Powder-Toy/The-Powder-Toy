#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <vector>
#include "SearchController.h"
#include "interface/SaveButton.h"
#include "interface/Button.h"

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
public:
	void NotifySaveListChanged(SearchModel * sender);
    SearchView();
	virtual ~SearchView();
	void AttachController(SearchController * _c) { c = _c; }
};

#endif // SEARCHVIEW_H
