#include "SearchModel.h"
#include "Save.h"

#include "client/Client.h"

SearchModel::SearchModel():
	currentSort("votes"),
	showOwn(false),
	loadedSave(NULL)
{
}

void SearchModel::UpdateSaveList(int pageNumber, std::string query)
{
	lastQuery = query;
	lastError = "";
	saveList.clear();
	currentPage = 1;
	resultCount = 0;
	notifySaveListChanged();
	notifyPageChanged();
	vector<Save*> * tempSaveList = Client::Ref().SearchSaves((pageNumber-1)*20, 20, query, currentSort, resultCount);
	saveList = *tempSaveList;
	delete tempSaveList;
	if(!saveList.size())
	{
		lastError = Client::Ref().GetLastError();
	}
	currentPage = pageNumber;
	notifyPageChanged();
	notifySaveListChanged();
}

void SearchModel::SetLoadedSave(Save * save)
{
	loadedSave = save;
}

Save * SearchModel::GetLoadedSave(){
	return loadedSave;
}

vector<Save*> SearchModel::GetSaveList()
{
	return saveList;
}

void SearchModel::AddObserver(SearchView * observer)
{
	observers.push_back(observer);
	observer->NotifySaveListChanged(this);
	observer->NotifyPageChanged(this);
	observer->NotifySortChanged(this);
	observer->NotifyShowOwnChanged(this);
}

void SearchModel::notifySaveListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySaveListChanged(this);
	}
}

void SearchModel::notifyPageChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyPageChanged(this);
	}
}

void SearchModel::notifySortChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySortChanged(this);
	}
}

void SearchModel::notifyShowOwnChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyShowOwnChanged(this);
	}
}

SearchModel::~SearchModel()
{
	if(loadedSave)
		delete loadedSave;
}
