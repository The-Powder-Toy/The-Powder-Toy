#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include <string>
#include <math.h>
#include "Save.h"
#include "SearchView.h"

using namespace std;

class SearchView;
class SearchModel
{
private:
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
public:
    SearchModel();
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
};

#endif // SEARCHMODEL_H
