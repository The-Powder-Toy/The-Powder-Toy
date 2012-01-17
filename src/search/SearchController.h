#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "interface/Panel.h"
#include "SearchModel.h"
#include "SearchView.h"

class SearchController
{
private:
	SearchModel * searchModel;
	SearchView * searchView;
	ui::Panel * windowPanel;
public:
	SearchController();
};

#endif // SEARCHCONTROLLER_H
