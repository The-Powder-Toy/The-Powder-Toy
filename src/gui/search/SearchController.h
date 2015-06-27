#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "gui/interface/Panel.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "gui/preview/PreviewController.h"
#include "Controller.h"
#include "client/SaveInfo.h"

class SearchView;
class SearchModel;
class SearchController
{
private:
	SearchModel * searchModel;
	SearchView * searchView;
	PreviewController * activePreview;
	ControllerCallback * callback;

	double nextQueryTime;
	std::string nextQuery;
	bool nextQueryDone;
	bool instantOpen;
	bool doRefresh;
	void removeSelectedC();
	void unpublishSelectedC(bool publish);
public:
	class OpenCallback;
	bool HasExited;
	SearchController(ControllerCallback * callback = NULL);
	~SearchController();
	SearchView * GetView() { return searchView; }
	void Exit();
	void DoSearch(std::string query, bool now = false);
	void DoSearch2(std::string query);
	void Refresh();
	void NextPage();
	void PrevPage();
	void SetPage(int page);
	void ChangeSort();
	void ShowOwn(bool show);
	void ShowFavourite(bool show);
	void Selected(int saveID, bool selected);
	void InstantOpen(bool instant);
	void OpenSave(int saveID);
	void OpenSave(int saveID, int saveDate);
	void Update();
	void ClearSelection();
	void RemoveSelected();
	void UnpublishSelected(bool publish);
	void FavouriteSelected();
	void ReleaseLoadedSave();
	SaveInfo * GetLoadedSave();
};

#endif // SEARCHCONTROLLER_H
