#pragma once
#include "gui/interface/Window.h"
#include <vector>

namespace ui
{
	class Label;
	class Textbox;
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
	ui::Label * pageLabel;
	ui::Label * pageCountLabel;
	ui::Textbox * pageTextbox;
	ui::Button * removeSelected;
	ui::Button *renameSelected;

	void textChanged();
	bool changed;
	unsigned int lastChanged;
	int pageCount;
public:
	LocalBrowserView();
	//virtual void OnDraw();
	void OnTick() override;
	void AttachController(LocalBrowserController * c_) { c = c_; }
	void NotifyPageChanged(LocalBrowserModel * sender);
	void NotifySavesListChanged(LocalBrowserModel * sender);
	void NotifySelectedChanged(LocalBrowserModel * sender);
	void OnMouseWheel(int x, int y, int d) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	virtual ~LocalBrowserView();
};
