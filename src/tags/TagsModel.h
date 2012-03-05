/*
 * TagsModel.h
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#ifndef TAGSMODEL_H_
#define TAGSMODEL_H_

#include <vector>

class TagsView;
class TagsModel {
	std::vector<TagsView*> observers;
public:
	TagsModel();
	void AddObserver(TagsView * observer);
	virtual ~TagsModel();
};

#endif /* TAGSMODEL_H_ */
