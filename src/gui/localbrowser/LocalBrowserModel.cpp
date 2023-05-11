#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"
#include "client/Client.h"
#include "client/SaveFile.h"
#include "client/GameSave.h"
#include "common/tpt-minmax.h"
#include <algorithm>

constexpr auto pageSize = 20;

LocalBrowserModel::LocalBrowserModel():
	currentPage(1),
	stampToFront(1)
{
	stampIDs = Client::Ref().GetStamps();
}


std::vector<SaveFile *> LocalBrowserModel::GetSavesList() // non-owning
{
	std::vector<SaveFile *> nonOwningSaveList;
	std::transform(savesList.begin(), savesList.end(), std::back_inserter(nonOwningSaveList), [](auto &ptr) {
		return ptr.get();
	});
	return nonOwningSaveList;
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

const SaveFile *LocalBrowserModel::GetSave()
{
	return stamp.get();
}

std::unique_ptr<SaveFile> LocalBrowserModel::TakeSave()
{
	return std::move(stamp);
}

void LocalBrowserModel::OpenSave(int index)
{
	stamp = std::move(savesList[index]);
	savesList.clear();
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
	savesList.clear();
	currentPage = pageNumber;
	notifyPageChanged();
	notifySavesListChanged();

	stampIDs = Client::Ref().GetStamps();
	auto size = int(stampIDs.size());
	for (int i = (currentPage - 1) * pageSize; i < size && i < currentPage * pageSize; i++)
	{
		auto tempSave = Client::Ref().GetStamp(stampIDs[i]);
		if (tempSave)
		{
			savesList.push_back(std::move(tempSave));
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
	auto size = int(stampIDs.size());
	return size / pageSize + ((size % pageSize) ? 1 : 0);
}

void LocalBrowserModel::SelectSave(ByteString stampID)
{
	if (std::find(selected.begin(), selected.end(), stampID) != selected.end())
	{
		return;
	}
	selected.push_back(stampID);
	notifySelectedChanged();
}

void LocalBrowserModel::DeselectSave(ByteString stampID)
{
	auto it = std::remove(selected.begin(), selected.end(), stampID);
	if (it != selected.end())
	{
		selected.erase(it, selected.end());
		notifySelectedChanged();
	}
}

void LocalBrowserModel::notifySelectedChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		LocalBrowserView* cObserver = observers[i];
		cObserver->NotifySelectedChanged(this);
	}
}
