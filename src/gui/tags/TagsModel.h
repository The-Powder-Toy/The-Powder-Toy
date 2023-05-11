#pragma once
#include "common/String.h"
#include <vector>

class SaveInfo;

class TagsView;
class TagsModel {
	SaveInfo *save = nullptr; // non-owning
	std::vector<TagsView*> observers;
	void notifyTagsChanged();
public:
	void AddObserver(TagsView * observer);
	void SetSave(SaveInfo *newSave /* non-owning */);
	void RemoveTag(ByteString tag);
	void AddTag(ByteString tag);
	SaveInfo *GetSave(); // non-owning
};
