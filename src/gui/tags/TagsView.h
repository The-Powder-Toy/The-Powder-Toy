#ifndef TAGSVIEW_H_
#define TAGSVIEW_H_

#include <vector>
#include "gui/interface/Window.h"

namespace ui
{
	class Button;
	class Textbox;
	class Label;
}

class TagsController;
class TagsModel;
class TagsView: public ui::Window {
	TagsController * c;
	ui::Button * addButton;
	ui::Button * closeButton;
	ui::Label * title;
	ui::Textbox * tagInput;
	std::vector<ui::Component*> tags;
	void addTag();
public:
	TagsView();
	virtual void OnDraw();
	void AttachController(TagsController * c_) { c = c_; }
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	void NotifyTagsChanged(TagsModel * sender);
	virtual ~TagsView();
};

#endif /* TAGSVIEW_H_ */
