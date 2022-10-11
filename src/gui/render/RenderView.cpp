#include "RenderView.h"

#include "simulation/ElementGraphics.h"

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "RenderController.h"
#include "RenderModel.h"

#include "gui/interface/Checkbox.h"
#include "gui/interface/Button.h"

class ModeCheckbox : public ui::Checkbox
{
public:
	using ui::Checkbox::Checkbox;
	unsigned int mode;
};

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, WINDOWH)),
	ren(NULL),
	toolTip(""),
	toolTipPresence(0),
	isToolTipFadingIn(false)
{
	auto addPresetButton = [this](int index, Icon icon, ui::Point offset, String tooltip) {
		auto *presetButton = new ui::Button(ui::Point(XRES, YRES) + offset, ui::Point(30, 13), "", tooltip);
		presetButton->SetIcon(icon);
		presetButton->SetActionCallback({ [this, index] { c->LoadRenderPreset(index); } });
		AddComponent(presetButton);
	};
	addPresetButton( 1, IconVelocity  , ui::Point( -37,  6), "Velocity display mode preset");
	addPresetButton( 2, IconPressure  , ui::Point( -37, 24), "Pressure display mode preset");
	addPresetButton( 3, IconPersistant, ui::Point( -76,  6), "Persistent display mode preset");
	addPresetButton( 4, IconFire      , ui::Point( -76, 24), "Fire display mode preset");
	addPresetButton( 5, IconBlob      , ui::Point(-115,  6), "Blob display mode preset");
	addPresetButton( 6, IconHeat      , ui::Point(-115, 24), "Heat display mode preset");
	addPresetButton( 7, IconBlur      , ui::Point(-154,  6), "Fancy display mode preset");
	addPresetButton( 8, IconBasic     , ui::Point(-154, 24), "Nothing display mode preset");
	addPresetButton( 9, IconGradient  , ui::Point(-193,  6), "Heat gradient display mode preset");
	addPresetButton( 0, IconAltAir    , ui::Point(-193, 24), "Alternative Velocity display mode preset");
	addPresetButton(10, IconLife      , ui::Point(-232,  6), "Life display mode preset");

	auto addRenderModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *renderModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		renderModes.push_back(renderModeCheckbox);
		renderModeCheckbox->mode = mode;
		renderModeCheckbox->SetIcon(icon);
		renderModeCheckbox->SetActionCallback({ [this, renderModeCheckbox] {
			if (renderModeCheckbox->GetChecked())
				c->SetRenderMode(renderModeCheckbox->mode);
			else
				c->UnsetRenderMode(renderModeCheckbox->mode);
		} });
		AddComponent(renderModeCheckbox);
	};
	addRenderModeCheckbox(RENDER_EFFE, IconEffect, ui::Point( 1,  4), "Adds Special flare effects to some elements");
	addRenderModeCheckbox(RENDER_FIRE, IconFire  , ui::Point( 1, 22), "Fire effect for gasses");
	addRenderModeCheckbox(RENDER_GLOW, IconGlow  , ui::Point(33,  4), "Glow effect on some elements");
	addRenderModeCheckbox(RENDER_BLUR, IconBlur  , ui::Point(33, 22), "Blur effect for liquids");
	addRenderModeCheckbox(RENDER_BLOB, IconBlob  , ui::Point(65,  4), "Makes everything be drawn like a blob");
	addRenderModeCheckbox(RENDER_BASC, IconBasic , ui::Point(65, 22), "Basic rendering, without this, most things will be invisible");
	addRenderModeCheckbox(RENDER_SPRK, IconEffect, ui::Point(97,  4), "Glow effect on sparks");

	auto addDisplayModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *displayModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		displayModes.push_back(displayModeCheckbox);
		displayModeCheckbox->mode = mode;
		displayModeCheckbox->SetIcon(icon);
		displayModeCheckbox->SetActionCallback({ [this, displayModeCheckbox] {
			if (displayModeCheckbox->GetChecked())
				c->SetDisplayMode(displayModeCheckbox->mode);
			else
				c->UnsetDisplayMode(displayModeCheckbox->mode);
		} });
		AddComponent(displayModeCheckbox);
	};
	line1 = 130;
	addDisplayModeCheckbox(DISPLAY_AIRC, IconAltAir    , ui::Point(135,  4), "Displays pressure as red and blue, and velocity as white");
	addDisplayModeCheckbox(DISPLAY_AIRP, IconPressure  , ui::Point(135, 22), "Displays pressure, red is positive and blue is negative");
	addDisplayModeCheckbox(DISPLAY_AIRV, IconVelocity  , ui::Point(167,  4), "Displays velocity and positive pressure: up/down adds blue, right/left adds red, still pressure adds green");
	addDisplayModeCheckbox(DISPLAY_AIRH, IconHeat      , ui::Point(167, 22), "Displays the temperature of the air like heat display does");
	line2 = 200;
	addDisplayModeCheckbox(DISPLAY_WARP, IconWarp      , ui::Point(205, 22), "Gravity lensing, Newtonian Gravity bends light with this on");
	addDisplayModeCheckbox(DISPLAY_EFFE, IconEffect    , ui::Point(205,  4), "Enables moving solids, stickmen guns, and premium(tm) graphics");
	addDisplayModeCheckbox(DISPLAY_PERS, IconPersistant, ui::Point(237,  4), "Element paths persist on the screen for a while");
	line3 = 270;

	auto addColourModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *colourModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		colourModes.push_back(colourModeCheckbox);
		colourModeCheckbox->mode = mode;
		colourModeCheckbox->SetIcon(icon);
		colourModeCheckbox->SetActionCallback({ [this, colourModeCheckbox] {
			if(colourModeCheckbox->GetChecked())
				c->SetColourMode(colourModeCheckbox->mode);
			else
				c->SetColourMode(0);
		} });
		AddComponent(colourModeCheckbox);
	};
	addColourModeCheckbox(COLOUR_HEAT, IconHeat    , ui::Point(275,  4), "Displays temperatures of the elements, dark blue is coldest, pink is hottest");
	addColourModeCheckbox(COLOUR_LIFE, IconLife    , ui::Point(275, 22), "Displays the life value of elements in greyscale gradients");
	addColourModeCheckbox(COLOUR_GRAD, IconGradient, ui::Point(307, 22), "Changes colors of elements slightly to show heat diffusing through them");
	addColourModeCheckbox(COLOUR_BASC, IconBasic   , ui::Point(307,  4), "No special effects at all for anything, overrides all other options and deco");
	line4 = 340;
}

void RenderView::OnMouseDown(int x, int y, unsigned button)
{
	if(x > XRES || y < YRES)
		c->Exit();
}

void RenderView::OnTryExit(ExitMethod method)
{
	c->Exit();
}

void RenderView::NotifyRendererChanged(RenderModel * sender)
{
	ren = sender->GetRenderer();
}

void RenderView::NotifyRenderChanged(RenderModel * sender)
{
	for (size_t i = 0; i < renderModes.size(); i++)
	{
		//Compares bitmasks at the moment, this means that "Point" is always on when other options that depend on it are, this might confuse some users, TODO: get the full list and compare that?
		auto renderMode = renderModes[i]->mode;
		renderModes[i]->SetChecked(renderMode == (sender->GetRenderMode() & renderMode));
	}
}

void RenderView::NotifyDisplayChanged(RenderModel * sender)
{
	for (size_t i = 0; i < displayModes.size(); i++)
	{
		auto displayMode = displayModes[i]->mode;
		displayModes[i]->SetChecked(displayMode == (sender->GetDisplayMode() & displayMode));
	}
}

void RenderView::NotifyColourChanged(RenderModel * sender)
{
	for (size_t i = 0; i < colourModes.size(); i++)
	{
		auto colourMode = colourModes[i]->mode;
		colourModes[i]->SetChecked(colourMode == sender->GetColourMode());
	}
}

void RenderView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(-1, -1, WINDOWW+1, WINDOWH+1);
	if(ren)
	{
		ren->clearScreen(1.0f);
		ren->RenderBegin();
		ren->RenderEnd();
	}
	g->draw_line(0, YRES, XRES-1, YRES, 200, 200, 200, 255);
	g->draw_line(line1, YRES, line1, WINDOWH, 200, 200, 200, 255);
	g->draw_line(line2, YRES, line2, WINDOWH, 200, 200, 200, 255);
	g->draw_line(line3, YRES, line3, WINDOWH, 200, 200, 200, 255);
	g->draw_line(line4, YRES, line4, WINDOWH, 200, 200, 200, 255);
	g->draw_line(XRES, 0, XRES, WINDOWH, 255, 255, 255, 255);
	if(toolTipPresence && toolTip.length())
	{
		g->drawtext(6, Size.Y-MENUSIZE-12, toolTip, 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
	}
}

void RenderView::OnTick(float dt)
{
	if (isToolTipFadingIn)
	{
		isToolTipFadingIn = false;
		if(toolTipPresence < 120)
		{
			toolTipPresence += int(dt*2)>1?int(dt*2):2;
			if(toolTipPresence > 120)
				toolTipPresence = 120;
		}
	}
	if(toolTipPresence>0)
	{
		toolTipPresence -= int(dt)>0?int(dt):1;
		if(toolTipPresence<0)
			toolTipPresence = 0;
	}
}

void RenderView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (shift && key == '1')
		c->LoadRenderPreset(10);
	else if(key >= '0' && key <= '9')
	{
		c->LoadRenderPreset(key-'0');
	}
}

void RenderView::ToolTip(ui::Point senderPosition, String toolTip)
{
	this->toolTip = toolTip;
	this->isToolTipFadingIn = true;
}

RenderView::~RenderView() {
}
