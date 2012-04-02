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

class StampsController;
class StampsModel;
class StampsView: public ui::Window {
	StampsController * c;
	std::vector<ui::Component*> stampButtons;
	ui::Button * previousButton;
	ui::Button * nextButton;
	ui::Label * infoLabel;
public:
	StampsView();
	//virtual void OnDraw();
	virtual void OnTick(float dt);
	void AttachController(StampsController * c_) { c = c_; };
	void NotifyPageChanged(StampsModel * sender);
	void NotifyStampsListChanged(StampsModel * sender);
	virtual ~StampsView();
};

#endif /* STAMPSVIEW_H_ */
