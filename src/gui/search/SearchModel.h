#pragma once
#include "common/String.h"
#include "client/http/Request.h"
#include "Config.h"
#include <vector>
#include <atomic>
#include <memory>

class SaveInfo;
class SearchView;
class SearchModel
{
private:
	std::unique_ptr<http::Request> searchSaves;
	void BeginSearchSaves(int start, int count, String query, ByteString sort, ByteString category);
	std::vector<std::unique_ptr<SaveInfo>> EndSearchSaves();

	void BeginGetTags(int start, int count, String query);
	std::vector<std::pair<ByteString, int>> EndGetTags();
	std::unique_ptr<http::Request> getTags;

	std::unique_ptr<SaveInfo> loadedSave;
	ByteString currentSort;
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
	void SetSort(ByteString sort) { if(!searchSaves) { currentSort = sort; } notifySortChanged(); }
	ByteString GetSort() { return currentSort; }
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
