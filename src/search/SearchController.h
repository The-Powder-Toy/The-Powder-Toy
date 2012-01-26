#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "interface/Panel.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "preview/PreviewController.h"
#include "Controller.h"
#include "Save.h"

class SearchView;
class SearchModel;
class SearchController
{
private:
	SearchModel * searchModel;
	SearchView * searchView;
	PreviewController * activePreview;
	ControllerCallback * callback;
public:
	class OpenCallback;
	bool HasExited;
	SearchController(ControllerCallback * callback = NULL);
	~SearchController();
	SearchView * GetView() { return searchView; }
	void Exit();
	void DoSearch(std::string query);
	void NextPage();
	void PrevPage();
	void ChangeSort();
	void ShowOwn(bool show);
	void OpenSave(int saveID);
	void Update();
	Save * GetLoadedSave();
};

#endif // SEARCHCONTROLLER_H
