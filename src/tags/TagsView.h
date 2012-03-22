/*
 * TagsView.h
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#ifndef TAGSVIEW_H_
#define TAGSVIEW_H_

#include <vector>
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Textbox.h"
#include "interface/Label.h"

class TagsController;
class TagsModel;
class TagsView: public ui::Window {
	TagsController * c;
	ui::Button * submitButton;
	ui::Textbox * tagInput;
	std::vector<ui::Label*> tags;
public:
	TagsView();
	virtual void OnDraw();
	void AttachController(TagsController * c_) { c = c_; };
	void NotifyTagsChanged(TagsModel * sender);
	virtual ~TagsView();
};

#endif /* TAGSVIEW_H_ */
