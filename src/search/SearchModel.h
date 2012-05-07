#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include <string>
#include <pthread.h>
#include <cmath>
#include "Save.h"
#include "SearchView.h"

using namespace std;

class SearchView;
class SearchModel
{
private:
	Save * loadedSave;
	string currentSort;
	string lastQuery;
	string lastError;
	vector<int> selected;
	vector<SearchView*> observers;
	vector<Save*> saveList;
	int currentPage;
	int resultCount;
	bool showOwn;
	bool showFavourite;
	void notifySaveListChanged();
	void notifySelectedChanged();
	void notifyPageChanged();
	void notifySortChanged();
	void notifyShowOwnChanged();
	void notifyShowFavouriteChanged();

	//Variables and methods for backgroun save request
	bool saveListLoaded;
	bool updateSaveListWorking;
	volatile bool updateSaveListFinished;
	pthread_t updateSaveListThread;
	static void * updateSaveListTHelper(void * obj);
	void * updateSaveListT();
public:
    SearchModel();
    virtual ~SearchModel();

	void AddObserver(SearchView * observer);
	void UpdateSaveList(int pageNumber, std::string query);
	vector<Save*> GetSaveList();
	string GetLastError() { return lastError; }
	int GetPageCount() { return max(1, (int)(ceil(resultCount/16))); }
	int GetPageNum() { return currentPage; }
	std::string GetLastQuery() { return lastQuery; }
	void SetSort(string sort) { currentSort = sort; notifySortChanged(); }
	string GetSort() { return currentSort; }
	void SetShowOwn(bool show) { if(show!=showOwn) { showOwn = show; } notifyShowOwnChanged();  }
	bool GetShowOwn() { return showOwn; }
	void SetShowFavourite(bool show) { if(show!=showFavourite) { showFavourite = show; } notifyShowFavouriteChanged();  }
	bool GetShowFavourite() { return showFavourite; }
	void SetLoadedSave(Save * save);
	Save * GetLoadedSave();
	bool GetSavesLoaded() { return saveListLoaded; }
	vector<int> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(int saveID);
	void DeselectSave(int saveID);
	void Update();
};

#endif // SEARCHMODEL_H
