#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include "common/String.h"
#include "common/tpt-minmax.h"
#include "common/tpt-thread.h"
#include <cmath>
#include "client/SaveInfo.h"
#include "SearchView.h"

using namespace std;

class SearchView;
class SearchModel
{
private:
	SaveInfo * loadedSave;
	ByteString currentSort;
	String lastQuery;
	String lastError;
	vector<int> selected;
	vector<SearchView*> observers;
	vector<SaveInfo*> saveList;
	vector<pair<ByteString, int> > tagList;
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
	volatile bool updateSaveListFinished;
	pthread_t updateSaveListThread;
	TH_ENTRY_POINT static void * updateSaveListTHelper(void * obj);
	void * updateSaveListT();

	bool updateTagListWorking;
	volatile bool updateTagListFinished;
	pthread_t updateTagListThread;
	TH_ENTRY_POINT static void * updateTagListTHelper(void * obj);
	void * updateTagListT();
public:
    SearchModel();
    virtual ~SearchModel();

    void SetShowTags(bool show);
    bool GetShowTags();
	void AddObserver(SearchView * observer);
	bool UpdateSaveList(int pageNumber, String query);
	vector<SaveInfo*> GetSaveList();
	vector<pair<ByteString, int> > GetTagList();
	String GetLastError() { return lastError; }
	int GetPageCount()
	{
		if (!showOwn && !showFavourite && currentSort == "best" && lastQuery == "")
			return std::max(1, (int)(ceil(resultCount/20.0f))+1); //add one for front page (front page saves are repeated twice)
		else
			return std::max(1, (int)(ceil(resultCount/20.0f)));
	}
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
	vector<int> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(int saveID);
	void DeselectSave(int saveID);
	void Update();
};

#endif // SEARCHMODEL_H
