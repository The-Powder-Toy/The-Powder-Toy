#pragma once
#include "common/String.h"
#include "client/Search.h"
#include "Config.h"
#include <vector>
#include <atomic>
#include <memory>

namespace http
{
	class SearchSavesRequest;
	class SearchTagsRequest;
}

class SaveInfo;
class SearchView;
class SearchModel
{
private:
	std::unique_ptr<http::SearchSavesRequest> searchSaves;
	void BeginSearchSaves(int start, int count, String query, http::Period period, http::Sort sort, http::Category category);
	std::vector<std::unique_ptr<SaveInfo>> EndSearchSaves();

	void BeginGetTags(int start, int count, String query);
	std::vector<std::pair<ByteString, int>> EndGetTags();
	std::unique_ptr<http::SearchTagsRequest> getTags;

	std::unique_ptr<SaveInfo> loadedSave;
	http::Period currentPeriod;
	http::Sort currentSort;
	String lastQuery;
	String lastError;
	std::vector<int> selected;
	std::vector<SearchView*> observers;
	std::vector<std::unique_ptr<SaveInfo>> saveList;
	std::vector<std::pair<ByteString, int> > tagList;
	int currentPage;
	int resultCount;
	bool showOwn;
	bool showFavourite;
	bool showTags;
	void notifySaveListChanged();
	void notifyTagListChanged();
	void notifySelectedChanged();
	void notifyPageChanged();
	void notifyPeriodChanged();
	void notifySortChanged();
	void notifyShowOwnChanged();
	void notifyShowFavouriteChanged();

	//Variables and methods for background save request
	bool saveListLoaded = false;
public:
    SearchModel();

    void SetShowTags(bool show);
    bool GetShowTags();
	void AddObserver(SearchView * observer);
	bool UpdateSaveList(int pageNumber, String query);
	std::vector<SaveInfo *> GetSaveList(); // non-owning
	std::vector<std::pair<ByteString, int> > GetTagList();
	String GetLastError() { return lastError; }
	int GetPageCount();
	int GetPageNum() { return currentPage; }
	String GetLastQuery() { return lastQuery; }
	void SetPeriod(http::Period period) { if(!searchSaves) { currentPeriod = period; } notifyPeriodChanged(); }
	http::Period GetPeriod() { return currentPeriod; }
	void SetSort(http::Sort sort) { if(!searchSaves) { currentSort = sort; } notifySortChanged(); }
	http::Sort GetSort() { return currentSort; }
	void SetShowOwn(bool show) { if(!searchSaves) { if(show!=showOwn) { showOwn = show; } } notifyShowOwnChanged();  }
	bool GetShowOwn() { return showOwn; }
	void SetShowFavourite(bool show) { if(show!=showFavourite && !searchSaves) { showFavourite = show; } notifyShowFavouriteChanged();  }
	bool GetShowFavourite() { return showFavourite; }
	void SetLoadedSave(std::unique_ptr<SaveInfo> save);
	const SaveInfo *GetLoadedSave() const;
	std::unique_ptr<SaveInfo> TakeLoadedSave();
	bool GetSavesLoaded() { return saveListLoaded; }
	std::vector<int> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(int saveID);
	void SelectAllSaves();
	void DeselectSave(int saveID);
	void Update();
};
