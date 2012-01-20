#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <vector>
#include <string>
#include "Save.h"
#include "SearchView.h"

using namespace std;

class SearchView;
class SearchModel
{
private:
	string lastError;
	vector<SearchView*> observers;
	vector<Save> saveList;
	void notifySaveListChanged();
public:
    SearchModel();
	void AddObserver(SearchView * observer);
	void UpdateSaveList(std::string query);
	vector<Save> GetSaveList();
	string GetLastError() { return lastError; }
};

#endif // SEARCHMODEL_H
