/*
 * OptionsView.h
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

#ifndef OPTIONSVIEW_H_
#define OPTIONSVIEW_H_

#include "interface/Window.h"
#include "OptionsController.h"
#include "interface/Checkbox.h"
#include "interface/DropDown.h"
#include "OptionsModel.h"

class OptionsModel;
class OptionsController;
class OptionsView: public ui::Window {
	OptionsController * c;
	ui::Checkbox * heatSimulation;
	ui::Checkbox * ambientHeatSimulation;
	ui::Checkbox * newtonianGravity;
	ui::Checkbox * waterEqualisation;
	ui::DropDown * airMode;
	ui::DropDown * gravityMode;
	ui::DropDown * edgeMode;
public:
	OptionsView();
	void NotifySettingsChanged(OptionsModel * sender);
	void AttachController(OptionsController * c_);
	void OnDraw();
	virtual ~OptionsView();
};

#endif /* OPTIONSVIEW_H_ */
