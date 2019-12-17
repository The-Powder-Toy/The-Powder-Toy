#ifndef TAGSCONTROLLER_H_
#define TAGSCONTROLLER_H_
#include "Config.h"

#include "common/String.h"

#include <functional>

class SaveInfo;
class TagsView;
class TagsModel;
class TagsController
{
	std::function<void ()> onDone;
	TagsView * tagsView;
	TagsModel * tagsModel;
public:
	bool HasDone;
	TagsController(std::function<void ()> onDone, SaveInfo * save);
	TagsView * GetView() {return tagsView;}
	SaveInfo * GetSave();
	void RemoveTag(ByteString tag);
	void AddTag(ByteString tag);
	void Exit();
	virtual ~TagsController();
};

#endif /* TAGSCONTROLLER_H_ */
