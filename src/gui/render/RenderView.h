#ifndef RENDERVIEW_H_
#define RENDERVIEW_H_


#include <vector>
#include "gui/interface/Window.h"
#include "RenderController.h"
#include "RenderModel.h"
#include "graphics/Renderer.h"
#include "gui/interface/Checkbox.h"
#include "gui/interface/Button.h"

class RenderController;
class RenderModel;
class RenderView: public ui::Window {
	RenderController * c;
	Renderer * ren;
	std::vector<ui::Checkbox*> renderModes;
	std::vector<ui::Checkbox*> displayModes;
	std::vector<ui::Checkbox*> colourModes;
	std::string toolTip;
	int toolTipPresence;
	bool isToolTipFadingIn;
	int line1, line2, line3, line4;
public:
	class RenderModeAction;
	class DisplayModeAction;
	class ColourModeAction;
	class RenderPresetAction;
	RenderView();
	void NotifyRendererChanged(RenderModel * sender);
	void NotifyRenderChanged(RenderModel * sender);
	void NotifyDisplayChanged(RenderModel * sender);
	void NotifyColourChanged(RenderModel * sender);
	void AttachController(RenderController * c_) { c = c_; }
	void OnMouseDown(int x, int y, unsigned button);
	void OnTryExit(ExitMethod method);
	virtual void OnDraw();
	virtual void OnTick(float dt);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void ToolTip(ui::Point senderPosition, std::string toolTip);
	virtual ~RenderView();
};

#endif /* RENDERVIEW_H_ */
