#include "SearchModel.h"
#include "Save.h"

#include "client/Client.h"

SearchModel::SearchModel():
	currentSort("best"),
	showOwn(false),
	loadedSave(NULL),
	updateSaveListWorking(false),
	updateSaveListFinished(false),
	saveListLoaded(false),
	currentPage(1),
	resultCount(0)
{
}

void * SearchModel::updateSaveListTHelper(void * obj)
{
	return ((SearchModel *)obj)->updateSaveListT();
}

void * SearchModel::updateSaveListT()
{
	vector<Save*> * tempSaveList = Client::Ref().SearchSaves((currentPage-1)*20, 20, lastQuery, currentSort=="new"?"date":"votes", showOwn, resultCount);
	updateSaveListFinished = true;
	return tempSaveList;
}

void SearchModel::UpdateSaveList(int pageNumber, std::string query)
{
	lastQuery = query;
	lastError = "";
	saveListLoaded = false;
	saveList.clear();
	//resultCount = 0;
	currentPage = pageNumber;
	notifySaveListChanged();
	notifyPageChanged();

	//Threading
	if(!updateSaveListWorking)
	{
		updateSaveListFinished = false;
		updateSaveListWorking = true;
		pthread_create(&updateSaveListThread, 0, &SearchModel::updateSaveListTHelper, this);
	}
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

void SearchModel::Update()
{
	if(updateSaveListWorking)
	{
		if(updateSaveListFinished)
		{
			updateSaveListWorking = false;
			lastError = "";
			saveListLoaded = true;
			vector<Save*> * tempSaveList;
			pthread_join(updateSaveListThread, (void**)(&tempSaveList));
			saveList = *tempSaveList;
			delete tempSaveList;
			if(!saveList.size())
			{
				lastError = Client::Ref().GetLastError();
			}
			//currentPage = pageNumber;
			notifyPageChanged();
			notifySaveListChanged();
		}
	}
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
