#ifndef OPTIONSVIEW_H_
#define OPTIONSVIEW_H_

#include "gui/interface/Window.h"
#include "gui/interface/ScrollPanel.h"

namespace ui
{
	class Checkbox;
	class DropDown;
	class Textbox;
	class Button;
}

class OptionsModel;
class OptionsController;
class OptionsView: public ui::Window
{
	OptionsController * c;
	ui::Checkbox * heatSimulation;
	ui::Checkbox * ambientHeatSimulation;
	ui::Checkbox * newtonianGravity;
	ui::Checkbox * waterEqualisation;
	ui::DropDown * airMode;
	ui::Textbox * ambientAirTemp;
	ui::Button * ambientAirTempPreview;
	ui::DropDown * gravityMode;
	ui::DropDown * edgeMode;
	ui::DropDown * scale;
	ui::Checkbox * resizable;
	ui::Checkbox * fullscreen;
	ui::Checkbox * altFullscreen;
	ui::Checkbox * forceIntegerScaling;
	ui::Checkbox * fastquit;
	ui::DropDown * decoSpace;
	ui::Checkbox * showAvatars;
	ui::Checkbox * momentumScroll;
	ui::Checkbox * mouseClickRequired;
	ui::Checkbox * includePressure;
	ui::Checkbox * perfectCirclePressure;
	ui::ScrollPanel * scrollPanel;
	bool ambientAirTempPreviewValid;
	float ambientAirTempPreviewValue;
	void UpdateAmbientAirTempPreview();
public:
	OptionsView();
	void NotifySettingsChanged(OptionsModel * sender);
	void AttachController(OptionsController * c_);
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;
	virtual ~OptionsView();
};

#endif /* OPTIONSVIEW_H_ */
