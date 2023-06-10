#include "TagsModel.h"
#include "TagsView.h"
#include "TagsModelException.h"
#include "client/Client.h"
#include "client/SaveInfo.h"
#include "client/http/AddTagRequest.h"
#include "client/http/RemoveTagRequest.h"
#include "gui/dialogues/ErrorMessage.h"

void TagsModel::SetSave(SaveInfo *newSave /* non-owning */)
{
	queuedTags.clear();
	this->save = newSave;
	notifyTagsChanged();
}

SaveInfo *TagsModel::GetSave() // non-owning
{
	return save;
}

void TagsModel::Tick()
{
	auto triggerTags = false;
	std::list<ByteString> tags;
	if (addTagRequest && addTagRequest->CheckDone())
	{
		try
		{
			tags = addTagRequest->Finish();
			triggerTags = true;
		}
		catch (const http::RequestError &ex)
		{
			new ErrorMessage("Could not add tag", ByteString(ex.what()).FromUtf8());
		}
		addTagRequest.reset();
	}
	if (removeTagRequest && removeTagRequest->CheckDone())
	{
		try
		{
			tags = removeTagRequest->Finish();
			triggerTags = true;
		}
		catch (const http::RequestError &ex)
		{
			new ErrorMessage("Could not remove tag", ByteString(ex.what()).FromUtf8());
		}
		removeTagRequest.reset();
	}
	if (triggerTags)
	{
		if (save)
		{
			save->SetTags(tags);
		}
		notifyTagsChanged();
	}
	if (!addTagRequest && !removeTagRequest && !queuedTags.empty())
	{
		auto it = queuedTags.begin();
		auto [ tag, add ] = *it;
		queuedTags.erase(it);
		if (save)
		{
			if (add)
			{
				addTagRequest = std::make_unique<http::AddTagRequest>(save->GetID(), tag);
				addTagRequest->Start();
			}
			else
			{
				removeTagRequest = std::make_unique<http::RemoveTagRequest>(save->GetID(), tag);
				removeTagRequest->Start();
			}
		}
	}
}

void TagsModel::RemoveTag(ByteString tag)
{
	queuedTags[tag] = false;
}

void TagsModel::AddTag(ByteString tag)
{
	queuedTags[tag] = true;
}

void TagsModel::AddObserver(TagsView * observer)
{
	observers.push_back(observer);
	observer->NotifyTagsChanged(this);
}

void TagsModel::notifyTagsChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyTagsChanged(this);
	}
}
