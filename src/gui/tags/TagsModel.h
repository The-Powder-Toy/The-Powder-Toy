#ifndef TAGSMODEL_H_
#define TAGSMODEL_H_
#include "Config.h"

#include <vector>
#include "common/String.h"

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

#endif /* TAGSMODEL_H_ */
