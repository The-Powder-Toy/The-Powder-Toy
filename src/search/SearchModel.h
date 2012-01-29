#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include <string>
#include <pthread.h>
#include <math.h>
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
	vector<SearchView*> observers;
	vector<Save*> saveList;
	int currentPage;
	int resultCount;
	bool showOwn;
	void notifySaveListChanged();
	void notifyPageChanged();
	void notifySortChanged();
	void notifyShowOwnChanged();

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
	void SetSort(string sort) { currentSort = sort; UpdateSaveList(1, lastQuery); notifySortChanged(); }
	string GetSort() { return currentSort; }
	void SetShowOwn(bool show) { showOwn = show; UpdateSaveList(1, lastQuery); notifyShowOwnChanged(); }
	bool GetShowOwn() { return showOwn; }
	void SetLoadedSave(Save * save);
	Save * GetLoadedSave();
	bool GetSavesLoaded() { return saveListLoaded; }
	void Update();
};

#endif // SEARCHMODEL_H
