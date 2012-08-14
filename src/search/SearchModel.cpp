#include "SearchModel.h"
#include "client/SaveInfo.h"

#include "client/Client.h"

SearchModel::SearchModel():
	currentSort("best"),
	showOwn(false),
	showFavourite(false),
	loadedSave(NULL),
	updateSaveListWorking(false),
	updateSaveListFinished(false),
	saveListLoaded(false),
	currentPage(1),
	resultCount(0),
	showTags(true)
{
}

void SearchModel::SetShowTags(bool show)
{
	showTags = show;
}

void * SearchModel::updateSaveListTHelper(void * obj)
{
	return ((SearchModel *)obj)->updateSaveListT();
}

void * SearchModel::updateSaveListT()
{
	void ** information = new void*[2];
	
	std::string category = "";
	if(showFavourite)
		category = "Favourites";
	if(showOwn && Client::Ref().GetAuthUser().ID)
		category = "by:"+Client::Ref().GetAuthUser().Username;
	information[0] = Client::Ref().SearchSaves((currentPage-1)*20, 20, lastQuery, currentSort=="new"?"date":"votes", category, thResultCount);

	if(showTags)
	{
		int tagResultCount;
		information[1] = Client::Ref().GetTags(0, 24, "", tagResultCount);
	}
	else
	{
		information[1] = NULL;
	}

	updateSaveListFinished = true;
	return information;
}

void SearchModel::UpdateSaveList(int pageNumber, std::string query)
{
	//Threading
	if(!updateSaveListWorking)
	{
		lastQuery = query;
		lastError = "";
		saveListLoaded = false;
		saveList.clear();
		//resultCount = 0;
		currentPage = pageNumber;
		notifySaveListChanged();
		notifyPageChanged();
		selected.clear();
		notifySelectedChanged();

		if(pageNumber == 1 && !showOwn && !showFavourite && currentSort == "best" && query == "")
			SetShowTags(true);
		else
			SetShowTags(false);
		
		updateSaveListFinished = false;
		updateSaveListWorking = true;
		pthread_create(&updateSaveListThread, 0, &SearchModel::updateSaveListTHelper, this);
	}
}

void SearchModel::SetLoadedSave(SaveInfo * save)
{
	if(loadedSave != save && loadedSave)
		delete loadedSave;
	if(save)
	{
		loadedSave = new SaveInfo(*save);
	}
	else
	{
		loadedSave = NULL;
	}
}

SaveInfo * SearchModel::GetLoadedSave(){
	return loadedSave;
}

vector<SaveInfo*> SearchModel::GetSaveList()
{
	return saveList;
}

vector<pair<string, int> > SearchModel::GetTagList()
{
	return tagList;
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
			void ** tempInformation;
			//vector<SaveInfo*> * tempSaveList;
			pthread_join(updateSaveListThread, (void**)(&tempInformation));


			saveList = *(vector<SaveInfo*>*)tempInformation[0];

			delete (vector<SaveInfo*>*)tempInformation[0];

			if(tempInformation[1])
			{
				tagList = *(vector<pair<string, int> >*)tempInformation[1];
				delete (vector<pair<string, int> >*)tempInformation[1];
			}
			else
			{
				tagList = vector<pair<string, int> >();
			}

			delete[] tempInformation;

			if(!saveList.size())
			{
				lastError = Client::Ref().GetLastError();
			}
			
			resultCount = thResultCount;
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

void SearchModel::SelectSave(int saveID)
{
	for(int i = 0; i < selected.size(); i++)
	{
		if(selected[i]==saveID)
		{
			return;
		}
	}
	selected.push_back(saveID);
	notifySelectedChanged();
}

void SearchModel::DeselectSave(int saveID)
{
	bool changed = false;
restart:
	for(int i = 0; i < selected.size(); i++)
	{
		if(selected[i]==saveID)
		{
			selected.erase(selected.begin()+i);
			changed = true;
			goto restart; //Just ensure all cases are removed.
		}
	}
	if(changed)
		notifySelectedChanged();
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

void SearchModel::notifyShowFavouriteChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyShowOwnChanged(this);
	}
}

void SearchModel::notifySelectedChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySelectedChanged(this);
	}
}

SearchModel::~SearchModel()
{
	if(loadedSave)
		delete loadedSave;
}
