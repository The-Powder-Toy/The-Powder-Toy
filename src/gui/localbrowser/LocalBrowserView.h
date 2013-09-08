#ifndef STAMPSVIEW_H_
#define STAMPSVIEW_H_

#include <vector>
#include "gui/interface/Window.h"

namespace ui
{
	class Label;
	class Button;
	class SaveButton;
}

class LocalBrowserController;
class LocalBrowserModel;
class LocalBrowserView: public ui::Window {
	LocalBrowserController * c;
	std::vector<ui::SaveButton*> stampButtons;
	ui::Button * undeleteButton;
	ui::Button * previousButton;
	ui::Button * nextButton;
	ui::Label * infoLabel;
	ui::Button * removeSelected;
public:
	LocalBrowserView();
	//virtual void OnDraw();
	virtual void OnTick(float dt);
	void AttachController(LocalBrowserController * c_) { c = c_; };
	void NotifyPageChanged(LocalBrowserModel * sender);
	void NotifySavesListChanged(LocalBrowserModel * sender);
	void NotifySelectedChanged(LocalBrowserModel * sender);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual ~LocalBrowserView();
};

#endif /* STAMPSVIEW_H_ */
