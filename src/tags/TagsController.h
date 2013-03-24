#ifndef TAGSCONTROLLER_H_
#define TAGSCONTROLLER_H_

#include "Controller.h"
#include "TagsView.h"
#include "client/SaveInfo.h"

class TagsView;
class TagsModel;
class TagsController {
	ControllerCallback * callback;
	TagsView * tagsView;
	TagsModel * tagsModel;
public:
	bool HasDone;
	TagsController(ControllerCallback * callback, SaveInfo * save);
	TagsView * GetView() {return tagsView;}
	SaveInfo * GetSave();
	void RemoveTag(std::string tag);
	void AddTag(std::string tag);
	void Exit();
	virtual ~TagsController();
};

#endif /* TAGSCONTROLLER_H_ */
