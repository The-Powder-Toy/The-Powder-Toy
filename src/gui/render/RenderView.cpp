#include "simulation/ElementGraphics.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "RenderView.h"

class RenderView::RenderModeAction: public ui::CheckboxAction
{
	RenderView * v;
public:
	unsigned int renderMode;
	RenderModeAction(RenderView * v_, unsigned int renderMode_)
	{
		v = v_;
		renderMode = renderMode_;
	}
	virtual void ActionCallback(ui::Checkbox * sender)
	{
		if(sender->GetChecked())
			v->c->SetRenderMode(renderMode);
		else
			v->c->UnsetRenderMode(renderMode);
	}
};

class RenderView::DisplayModeAction: public ui::CheckboxAction
{
	RenderView * v;
public:
	unsigned int displayMode;
	DisplayModeAction(RenderView * v_, unsigned int displayMode_)
	{
		v = v_;
		displayMode = displayMode_;
	}
	virtual void ActionCallback(ui::Checkbox * sender)
	{
		if(sender->GetChecked())
			v->c->SetDisplayMode(displayMode);
		else
			v->c->UnsetDisplayMode(displayMode);
	}
};

class RenderView::ColourModeAction: public ui::CheckboxAction
{
	RenderView * v;
public:
	unsigned int colourMode;
	ColourModeAction(RenderView * v_, unsigned int colourMode_)
	{
		v = v_;
		colourMode = colourMode_;
	}
	virtual void ActionCallback(ui::Checkbox * sender)
	{
		if(sender->GetChecked())
			v->c->SetColourMode(colourMode);
		else
			v->c->SetColourMode(0);
	}
};

class RenderView::RenderPresetAction: public ui::ButtonAction
{
	RenderView * v;
public:
	int renderPreset;
	RenderPresetAction(RenderView * v_, int renderPreset_)
	{
		v = v_;
		renderPreset = renderPreset_;
	}
	virtual void ActionCallback(ui::Button * sender)
	{
		v->c->LoadRenderPreset(renderPreset);
	}
};

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, WINDOWH)),
	ren(NULL),
	toolTip(""),
	toolTipPresence(0),
	isToolTipFadingIn(false)
{
	ui::Button * presetButton;
	int presetButtonOffset = 375;
	int checkboxOffset = 1;
	int cSpace = 32;
	int sSpace = 38;

	presetButton = new ui::Button(ui::Point(presetButtonOffset+200, YRES+6), ui::Point(30, 13), "", "Velocity display mode preset");
	presetButton->SetIcon(IconVelocity);
	presetButton->SetActionCallback(new RenderPresetAction(this, 1));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+200, YRES+6+18), ui::Point(30, 13), "", "Pressure display mode preset");
	presetButton->SetIcon(IconPressure);
	presetButton->SetActionCallback(new RenderPresetAction(this, 2));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+161, YRES+6), ui::Point(30, 13), "", "Persistent display mode preset");
	presetButton->SetIcon(IconPersistant);
	presetButton->SetActionCallback(new RenderPresetAction(this, 3));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+161, YRES+6+18), ui::Point(30, 13), "", "Fire display mode preset");
	presetButton->SetIcon(IconFire);
	presetButton->SetActionCallback(new RenderPresetAction(this, 4));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+122, YRES+6), ui::Point(30, 13), "", "Blob display mode preset");
	presetButton->SetIcon(IconBlob);
	presetButton->SetActionCallback(new RenderPresetAction(this, 5));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+122, YRES+6+18), ui::Point(30, 13), "", "Heat display mode preset");
	presetButton->SetIcon(IconHeat);
	presetButton->SetActionCallback(new RenderPresetAction(this, 6));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+83, YRES+6), ui::Point(30, 13), "", "Fancy display mode preset");
	presetButton->SetIcon(IconBlur);
	presetButton->SetActionCallback(new RenderPresetAction(this, 7));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+83, YRES+6+18), ui::Point(30, 13), "", "Nothing display mode preset");
	presetButton->SetIcon(IconBasic);
	presetButton->SetActionCallback(new RenderPresetAction(this, 8));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+44, YRES+6), ui::Point(30, 13), "", "Heat gradient display mode preset");
	presetButton->SetIcon(IconGradient);
	presetButton->SetActionCallback(new RenderPresetAction(this, 9));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+44, YRES+6+18), ui::Point(30, 13), "", "Alternative Velocity display mode preset");
	presetButton->SetIcon(IconAltAir);
	presetButton->SetActionCallback(new RenderPresetAction(this, 0));
	AddComponent(presetButton);

	presetButton = new ui::Button(ui::Point(presetButtonOffset+5, YRES+6), ui::Point(30, 13), "", "Life display mode preset");
	presetButton->SetIcon(IconLife);
	presetButton->SetActionCallback(new RenderPresetAction(this, 10));
	AddComponent(presetButton);

	ui::Checkbox * tCheckbox;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Effects", "Adds Special flare effects to some elements");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconEffect);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_EFFE));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Fire", "Fire effect for gasses");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconFire);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_FIRE));
	AddComponent(tCheckbox);

	checkboxOffset += cSpace;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Glow", "Glow effect on some elements");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconGlow);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_GLOW));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Blur", "Blur effect for liquids");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconBlur);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BLUR));
	AddComponent(tCheckbox);

	checkboxOffset += cSpace;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Blob", "Makes everything be drawn like a blob");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconBlob);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BLOB));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Point", "Basic rendering, without this, most things will be invisible");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconBasic);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BASC));
	AddComponent(tCheckbox);

	checkboxOffset += cSpace;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Spark", "Glow effect on sparks");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconEffect);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_SPRK));
	AddComponent(tCheckbox);

	checkboxOffset += sSpace;
	line1 = checkboxOffset-5;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Alt. Air", "Displays pressure as red and blue, and velocity as white");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconAltAir);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRC));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Pressure", "Displays pressure, red is positive and blue is negative");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconPressure);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRP));
	AddComponent(tCheckbox);

	checkboxOffset += cSpace;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Velocity", "Displays velocity and positive pressure: up/down adds blue, right/left adds red, still pressure adds green");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconVelocity);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRV));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Air-heat", "Displays the temperature of the air like heat display does");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconHeat);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRH));
	AddComponent(tCheckbox);

	/*tCheckbox = new ui::Checkbox(ui::Point(216, YRES+4), ui::Point(30, 16), "Air", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconAltAir);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIR));
	AddComponent(tCheckbox);*/

	checkboxOffset += sSpace;
	line2 = checkboxOffset-5;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Warp", "Gravity lensing, Newtonian Gravity bends light with this on");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconWarp);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_WARP));
	AddComponent(tCheckbox);

#ifdef OGLR
	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Effect", "Some type of OpenGL effect ... maybe"); //I would remove the whole checkbox, but then there's a large empty space
#else
	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Effect", "Enables moving solids, stickmen guns, and premium(tm) graphics");
#endif
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconEffect);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_EFFE));
	AddComponent(tCheckbox);

	checkboxOffset += cSpace;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Persistent", "Element paths persist on the screen for a while");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconPersistant);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_PERS));
	AddComponent(tCheckbox);

	checkboxOffset += sSpace;
	line3 = checkboxOffset-5;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Heat", "Displays temperatures of the elements, dark blue is coldest, pink is hottest");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconHeat);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_HEAT));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "Life", "Displays the life value of elements in greyscale gradients");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconLife);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_LIFE));
	AddComponent(tCheckbox);

	checkboxOffset += cSpace;

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4+18), ui::Point(30, 16), "H-Gradient", "Changes colors of elements slightly to show heat diffusing through them");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconGradient);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_GRAD));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(checkboxOffset, YRES+4), ui::Point(30, 16), "Basic", "No special effects at all for anything, overrides all other options and deco");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetIcon(IconBasic);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_BASC));
	AddComponent(tCheckbox);

	checkboxOffset += sSpace;
	line4 = checkboxOffset-5;
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
		if (renderModes[i]->GetActionCallback())
		{
			//Compares bitmasks at the moment, this means that "Point" is always on when other options that depend on it are, this might confuse some users, TODO: get the full list and compare that?
			RenderModeAction * action = (RenderModeAction *)(renderModes[i]->GetActionCallback());
			if (action->renderMode  == (sender->GetRenderMode() & action->renderMode))
			{
				renderModes[i]->SetChecked(true);
			}
			else
			{
				renderModes[i]->SetChecked(false);
			}
		}
	}
}

void RenderView::NotifyDisplayChanged(RenderModel * sender)
{
	for (size_t i = 0; i < displayModes.size(); i++)
	{
		if( displayModes[i]->GetActionCallback())
		{
			DisplayModeAction * action = (DisplayModeAction *)(displayModes[i]->GetActionCallback());
			if (action->displayMode  == (sender->GetDisplayMode() & action->displayMode))
			{
				displayModes[i]->SetChecked(true);
			}
			else
			{
				displayModes[i]->SetChecked(false);
			}
		}
	}
}

void RenderView::NotifyColourChanged(RenderModel * sender)
{
	for (size_t i = 0; i < colourModes.size(); i++)
	{
		if (colourModes[i]->GetActionCallback())
		{
			ColourModeAction * action = (ColourModeAction *)(colourModes[i]->GetActionCallback());
			if (action->colourMode == sender->GetColourMode())
			{
				colourModes[i]->SetChecked(true);
			}
			else
			{
				colourModes[i]->SetChecked(false);
			}
		}
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
		g->drawtext(6, Size.Y-MENUSIZE-12, (char*)toolTip.c_str(), 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
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

void RenderView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (shift && key == '1')
		c->LoadRenderPreset(10);
	else if(key >= '0' && key <= '9')
	{
		c->LoadRenderPreset(key-'0');
	}
}

void RenderView::ToolTip(ui::Point senderPosition, std::string toolTip)
{
	this->toolTip = toolTip;
	this->isToolTipFadingIn = true;
}

RenderView::~RenderView() {
}
