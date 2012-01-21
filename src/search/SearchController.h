#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "interface/Panel.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "preview/PreviewController.h"
class SearchView;
class SearchModel;
class SearchController
{
private:
	SearchModel * searchModel;
	SearchView * searchView;
	PreviewController * activePreview;
public:
	SearchController();
	~SearchController();
	SearchView * GetView() { return searchView; }
	void DoSearch(std::string query);
	void NextPage();
	void PrevPage();
	void ChangeSort();
	void ShowOwn(bool show);
	void OpenSave(int saveID);
};

#endif // SEARCHCONTROLLER_H
