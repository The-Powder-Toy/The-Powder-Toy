#pragma once
#include "gui/interface/Window.h"
#include <cstdint>
#include <vector>

class ModeCheckbox;

class Renderer;
struct RendererSettings;
class Simulation;
class RenderController;
class RenderModel;
class RenderView: public ui::Window {
	RenderController * c;
	Renderer * ren;
	RendererSettings *rendererSettings = nullptr;
	Simulation *sim = nullptr;
	std::vector<ModeCheckbox *> renderModes;
	std::vector<ModeCheckbox *> displayModes;
	std::vector<ModeCheckbox *> colourModes;
	String toolTip;
	int toolTipPresence;
	bool isToolTipFadingIn;
	int line1, line2, line3, line4;
	uint32_t CalculateRenderMode();
public:
	RenderView();
	void NotifyRendererChanged(RenderModel * sender);
	void NotifySimulationChanged(RenderModel * sender);
	void NotifyRenderChanged(RenderModel * sender);
	void NotifyDisplayChanged(RenderModel * sender);
	void NotifyColourChanged(RenderModel * sender);
	void AttachController(RenderController * c_) { c = c_; }
	void OnMouseDown(int x, int y, unsigned button) override;
	void OnTryExit(ExitMethod method) override;
	void OnDraw() override;
	void OnTick(float dt) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void ToolTip(ui::Point senderPosition, String toolTip) override;
	virtual ~RenderView();
};
