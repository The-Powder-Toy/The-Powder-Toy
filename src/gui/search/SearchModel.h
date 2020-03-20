#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include "common/String.h"
#include <atomic>

class SaveInfo;
class SearchView;
class SearchModel
{
private:
	SaveInfo * loadedSave;
	ByteString currentSort;
	String lastQuery;
	String lastError;
	std::vector<int> selected;
	std::vector<SearchView*> observers;
	std::vector<SaveInfo*> saveList;
	std::vector<std::pair<ByteString, int> > tagList;
	int currentPage;
	int resultCount;
	int thResultCount;
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
	bool saveListLoaded;
	bool updateSaveListWorking;
	std::atomic<bool> updateSaveListFinished;
	void updateSaveListT();
	std::vector<SaveInfo *> *updateSaveListResult;

	bool updateTagListWorking;
	std::atomic<bool> updateTagListFinished;
	void updateTagListT();
	std::vector<std::pair<ByteString, int>> *updateTagListResult;
public:
    SearchModel();
    virtual ~SearchModel();

    void SetShowTags(bool show);
    bool GetShowTags();
	void AddObserver(SearchView * observer);
	bool UpdateSaveList(int pageNumber, String query);
	std::vector<SaveInfo*> GetSaveList();
	std::vector<std::pair<ByteString, int> > GetTagList();
	String GetLastError() { return lastError; }
	int GetPageCount();
	int GetPageNum() { return currentPage; }
	String GetLastQuery() { return lastQuery; }
	void SetSort(ByteString sort) { if(!updateSaveListWorking) { currentSort = sort; } notifySortChanged(); }
	ByteString GetSort() { return currentSort; }
	void SetShowOwn(bool show) { if(!updateSaveListWorking) { if(show!=showOwn) { showOwn = show; } } notifyShowOwnChanged();  }
	bool GetShowOwn() { return showOwn; }
	void SetShowFavourite(bool show) { if(show!=showFavourite && !updateSaveListWorking) { showFavourite = show; } notifyShowFavouriteChanged();  }
	bool GetShowFavourite() { return showFavourite; }
	void SetLoadedSave(SaveInfo * save);
	SaveInfo * GetLoadedSave();
	bool GetSavesLoaded() { return saveListLoaded; }
	std::vector<int> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(int saveID);
	void SelectAllSaves();
	void DeselectSave(int saveID);
	void Update();
};

#endif // SEARCHMODEL_H
