/*
 * TagsController.h
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#ifndef TAGSCONTROLLER_H_
#define TAGSCONTROLLER_H_

#include "Controller.h"
#include "TagsView.h"
#include "search/Save.h"

class TagsView;
class TagsModel;
class TagsController {
	ControllerCallback * callback;
	TagsView * tagsView;
	TagsModel * tagsModel;
public:
	bool HasDone;
	TagsController(ControllerCallback * callback, Save * save);
	TagsView * GetView() {return tagsView;}
	Save * GetSave();
	void Exit();
	virtual ~TagsController();
};

#endif /* TAGSCONTROLLER_H_ */
