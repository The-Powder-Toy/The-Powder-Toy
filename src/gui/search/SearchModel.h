#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include <string>
#include <pthread.h>
#undef GetUserName //God dammit microsoft!
#include <cmath>
#include "client/SaveInfo.h"
#include "SearchView.h"

using namespace std;

class SearchView;
class SearchModel
{
private:
	SaveInfo * loadedSave;
	string currentSort;
	string lastQuery;
	string lastError;
	vector<int> selected;
	vector<SearchView*> observers;
	vector<SaveInfo*> saveList;
	vector<pair<string, int> > tagList;
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
	static void * updateSaveListTHelper(void * obj);
	void * updateSaveListT();

	bool updateTagListWorking;
	volatile bool updateTagListFinished;
	pthread_t updateTagListThread;
	static void * updateTagListTHelper(void * obj);
	void * updateTagListT();
public:
    SearchModel();
    virtual ~SearchModel();

    void SetShowTags(bool show);
    bool GetShowTags();
	void AddObserver(SearchView * observer);
	void UpdateSaveList(int pageNumber, std::string query);
	vector<SaveInfo*> GetSaveList();
	vector<pair<string, int> > GetTagList();
	string GetLastError() { return lastError; }
	int GetPageCount()
	{
		if (!showOwn && !showFavourite && currentSort == "best" && lastQuery == "")
			return max(1, (int)(ceil((resultCount+5)/20.0f)));
		else
			return max(1, (int)(ceil(resultCount/20.0f)));
	}
	int GetPageNum() { return currentPage; }
	std::string GetLastQuery() { return lastQuery; }
	void SetSort(string sort) { if(!updateSaveListWorking) { currentSort = sort; } notifySortChanged(); }
	string GetSort() { return currentSort; }
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
