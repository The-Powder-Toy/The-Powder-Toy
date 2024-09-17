#pragma once
#include "common/String.h"
#include <functional>
#include <memory>

class SaveInfo;
class PreviewController;
class PreviewController;
class SearchView;
class SearchModel;
class VideoBuffer;
class SearchController
{
private:
	SearchModel * searchModel;
	SearchView * searchView;
	PreviewController * activePreview;
	std::function<void ()> onDone;

	double nextQueryTime;
	String nextQuery;
	bool nextQueryDone;
	bool instantOpen;
	bool doRefresh;
	void removeSelectedC();
	void unpublishSelectedC(bool publish);

	void OpenSaveDone();
public:
	bool HasExited;
	SearchController(std::function<void ()> onDone = nullptr);
	~SearchController();
	SearchView * GetView() { return searchView; }
	void Exit();
	void DoSearch(String query, bool now = false);
	void DoSearch2(const String& query);
	void Refresh();
	void SetPage(int page);
	void SetPageRelative(int offset);
	void ChangePeriod(int period);
	void ChangeSort();
	void ShowOwn(bool show);
	void ShowFavourite(bool show);
	void Selected(int saveID, bool selected);
	void SelectAllSaves();
	void InstantOpen(bool instant);
	void OpenSave(int saveID, int saveDate, std::unique_ptr<VideoBuffer> thumbnail);
	void Update();
	void ClearSelection();
	void RemoveSelected();
	void UnpublishSelected(bool publish);
	void FavouriteSelected();
	const SaveInfo *GetLoadedSave() const;
	std::unique_ptr<SaveInfo> TakeLoadedSave();
};
