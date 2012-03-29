/*
 * TagsModel.h
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#ifndef TAGSMODEL_H_
#define TAGSMODEL_H_

#include <vector>
#include "search/Save.h"

class TagsView;
class TagsModel {
	Save * save;
	std::vector<TagsView*> observers;
	void notifyTagsChanged();
public:
	TagsModel();
	void AddObserver(TagsView * observer);
	void SetSave(Save * save);
	void RemoveTag(string tag);
	void AddTag(string tag);
	Save * GetSave();
	virtual ~TagsModel();
};

#endif /* TAGSMODEL_H_ */
