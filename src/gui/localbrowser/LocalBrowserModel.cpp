#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"
#include "LocalBrowserModelException.h"
#include "client/Client.h"
#include "client/SaveFile.h"

LocalBrowserModel::LocalBrowserModel():
	stamp(NULL),
	currentPage(1),
	stampToFront(1)
{
	//stampIDs = Client::Ref().GetStamps();
	stampIDs = Client::Ref().GetStamps(0, 16);
}


std::vector<SaveFile*> LocalBrowserModel::GetSavesList()
{
	return savesList;
}

void LocalBrowserModel::AddObserver(LocalBrowserView * observer)
{
	observers.push_back(observer);
	observer->NotifySavesListChanged(this);
	observer->NotifyPageChanged(this);
}

void LocalBrowserModel::notifySavesListChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySavesListChanged(this);
		observers[i]->NotifyPageChanged(this);
	}
}

void LocalBrowserModel::notifyPageChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPageChanged(this);
	}
}

SaveFile * LocalBrowserModel::GetSave()
{
	return stamp;
}

void LocalBrowserModel::SetSave(SaveFile * newStamp)
{
	delete stamp;
	stamp = new SaveFile(*newStamp);
}

bool LocalBrowserModel::GetMoveToFront()
{
	return stampToFront;
}

void LocalBrowserModel::SetMoveToFront(bool move)
{
	stampToFront = move;
}

void LocalBrowserModel::UpdateSavesList(int pageNumber)
{
	std::vector<SaveFile*> tempSavesList = savesList;
	savesList.clear();
	currentPage = pageNumber;
	notifyPageChanged();
	notifySavesListChanged();
	//notifyStampsListChanged();
	/*for(int i = 0; i < tempSavesList.size(); i++)
	{
		delete tempSavesList[i];
	}*/

	stampIDs = Client::Ref().GetStamps((pageNumber-1)*20, 20);

	for (size_t i = 0; i < stampIDs.size(); i++)
	{
		SaveFile * tempSave = Client::Ref().GetStamp(stampIDs[i]);
		if (tempSave)
		{
			savesList.push_back(tempSave);
		}
	}
	notifySavesListChanged();
}

void LocalBrowserModel::RescanStamps()
{
	Client::Ref().RescanStamps();
}

int LocalBrowserModel::GetPageCount()
{
	return std::max(1, (int)(std::ceil(float(Client::Ref().GetStampsCount())/20.0f)));
}

void LocalBrowserModel::SelectSave(std::string stampID)
{
	for (size_t i = 0; i < selected.size(); i++)
	{
		if (selected[i] == stampID)
		{
			return;
		}
	}
	selected.push_back(stampID);
	notifySelectedChanged();
}

void LocalBrowserModel::DeselectSave(std::string stampID)
{
	bool changed = false;
restart:
	for (size_t i = 0; i < selected.size(); i++)
	{
		if (selected[i] == stampID)
		{
			selected.erase(selected.begin()+i);
			changed = true;
			goto restart; //Just ensure all cases are removed.
		}
	}
	if(changed)
		notifySelectedChanged();
}

void LocalBrowserModel::notifySelectedChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		LocalBrowserView* cObserver = observers[i];
		cObserver->NotifySelectedChanged(this);
	}
}

LocalBrowserModel::~LocalBrowserModel() {
	delete stamp;
}

