#ifndef TAGSMODEL_H_
#define TAGSMODEL_H_

#include <vector>
#include <string>

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
	void RemoveTag(std::string tag);
	void AddTag(std::string tag);
	SaveInfo * GetSave();
	virtual ~TagsModel();
};

#endif /* TAGSMODEL_H_ */
