/*
 * TagsModel.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#include "TagsModel.h"

TagsModel::TagsModel() {
	// TODO Auto-generated constructor stub

}

void TagsModel::AddObserver(TagsView * observer)
{
	observers.push_back(observer);
}

TagsModel::~TagsModel() {
	// TODO Auto-generated destructor stub
}

