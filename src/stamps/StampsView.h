/*
 * StampsView.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef STAMPSVIEW_H_
#define STAMPSVIEW_H_

#include <vector>
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Textbox.h"
#include "interface/Label.h"
#include "interface/SaveButton.h"

class StampsController;
class StampsModel;
class StampsView: public ui::Window {
	StampsController * c;
	std::vector<ui::SaveButton*> stampButtons;
	ui::Button * previousButton;
	ui::Button * nextButton;
	ui::Label * infoLabel;
	ui::Button * removeSelected;
public:
	StampsView();
	//virtual void OnDraw();
	virtual void OnTick(float dt);
	void AttachController(StampsController * c_) { c = c_; };
	void NotifyPageChanged(StampsModel * sender);
	void NotifyStampsListChanged(StampsModel * sender);
	void NotifySelectedChanged(StampsModel * sender);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual ~StampsView();
};

#endif /* STAMPSVIEW_H_ */
