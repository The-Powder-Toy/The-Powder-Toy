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
	ui::Checkbox * scale;
	ui::Checkbox * fullscreen;
	ui::Checkbox * fastquit;
	ui::Checkbox * showAvatars;
public:
	OptionsView();
	void NotifySettingsChanged(OptionsModel * sender);
	void AttachController(OptionsController * c_);
	void OnDraw();
	void OnTryExit(ExitMethod method);
	virtual ~OptionsView();
};

#endif /* OPTIONSVIEW_H_ */
