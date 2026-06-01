#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"
#include "client/Client.h"
#include "client/SaveFile.h"
#include "client/GameSave.h"
#include <algorithm>

constexpr auto pageSize = 20;

LocalBrowserModel::LocalBrowserModel()
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
	notifyPageChanged();
	notifySavesListChanged();
}

bool LocalBrowserModel::GetMoveToFront()
{
	return stampToFront;
}

void LocalBrowserModel::SetMoveToFront(bool move)
{
	stampToFront = move;
}

void LocalBrowserModel::UpdateSavesList(ByteString query, int pageNumber)
{
	ClearSelected();
	savesList.clear();
	currentQuery = query;
	currentPage = pageNumber;

	stampIDs = Client::Ref().GetStamps();
	int found = 0;
	for (auto &id : stampIDs)
	{
		if (!(!query.size() || id.Contains(query)))
		{
			continue;
		}
		if (found >= currentPage * pageSize && found < (currentPage + 1) * pageSize)
		{
			auto tempSave = Client::Ref().GetStamp(id);
			if (tempSave)
			{
				savesList.push_back(std::move(tempSave));
			}
		}
		found += 1;
	}
	notifyPageChanged();
	notifySavesListChanged();
}

void LocalBrowserModel::RescanStamps()
{
	Client::Ref().RescanStamps();
}

int LocalBrowserModel::GetPageCount()
{
	auto size = int(stampIDs.size());
	auto count = size / pageSize + ((size % pageSize) ? 1 : 0);
	return count ? count : 1; // there is always at least one page; there may not be anything on it though
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
