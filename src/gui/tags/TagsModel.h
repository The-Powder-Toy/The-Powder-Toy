#pragma once
#include "common/String.h"
#include <vector>

class SaveInfo;

class TagsView;
class TagsModel {
	SaveInfo * save;
	std::vector<TagsView*> observers;
	void notifyTagsChanged();
public:
	TagsModel();
	void AddObserver(TagsView * observer);
	void SetSave(SaveInfo * save);
	void RemoveTag(ByteString tag);
	void AddTag(ByteString tag);
	SaveInfo * GetSave();
	virtual ~TagsModel();
};
