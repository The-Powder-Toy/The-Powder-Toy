/*
 * TagsView.h
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#ifndef TAGSVIEW_H_
#define TAGSVIEW_H_

#include "interface/Window.h"

class TagsController;
class TagsModel;
class TagsView: public ui::Window {
	TagsController * c;
public:
	TagsView();
	virtual void OnDraw();
	void AttachController(TagsController * c_) { c = c_; };
	virtual ~TagsView();
};

#endif /* TAGSVIEW_H_ */
