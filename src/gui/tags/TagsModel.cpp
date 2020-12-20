#include "TagsModel.h"

#include "TagsView.h"
#include "TagsModelException.h"

#include "client/Client.h"
#include "client/SaveInfo.h"

TagsModel::TagsModel():
	save(NULL)
{

}

void TagsModel::SetSave(SaveInfo * save)
{
	this->save = save;
	notifyTagsChanged();
}

SaveInfo * TagsModel::GetSave()
{
	return save;
}

void TagsModel::RemoveTag(ByteString tag)
{
	if(save)
	{
		std::list<ByteString> * tags = Client::Ref().RemoveTag(save->GetID(), tag);
		if(tags)
		{
			save->SetTags(std::list<ByteString>(*tags));
			notifyTagsChanged();
			delete tags;
		}
		else
		{
			throw TagsModelException(Client::Ref().GetLastError());
		}
	}
}

void TagsModel::AddTag(ByteString tag)
{
	if(save)
	{
		std::list<ByteString> * tags = Client::Ref().AddTag(save->GetID(), tag);
		if(tags)
		{
			save->SetTags(std::list<ByteString>(*tags));
			notifyTagsChanged();
			delete tags;
		}
		else
		{
			throw TagsModelException(Client::Ref().GetLastError());
		}
	}
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

TagsModel::~TagsModel() {
}

