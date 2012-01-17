#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include "Save.h"
#include "SearchView.h"

using namespace std;

class SearchModel
{
private:
	vector<SearchView*> observers;
	vector<Save> saveList;
	void notifySaveListChanged();
public:
    SearchModel();
	void AddObserver(SearchView * observer){ observers.push_back(observer); }
	void UpdateSaveList();
	vector<Save> GetSaveList();
};

#endif // SEARCHMODEL_H
