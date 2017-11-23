#ifndef OPTIONSVIEW_H_
#define OPTIONSVIEW_H_

#include "gui/interface/Window.h"
#include "OptionsController.h"
#include "gui/interface/Checkbox.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Textbox.h"
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
	ui::DropDown * scale;
	ui::Checkbox * fullscreen;
	ui::Checkbox * fastquit;
	ui::Checkbox * showAvatars;
	ui::Textbox  * depthTextbox;
public:
	OptionsView();
	void NotifySettingsChanged(OptionsModel * sender);
	void AttachController(OptionsController * c_);
	void OnDraw();
	void OnTryExit(ExitMethod method);
	virtual ~OptionsView();
};

#endif /* OPTIONSVIEW_H_ */
