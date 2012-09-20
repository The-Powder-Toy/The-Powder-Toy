/*
 * RenderView.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

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

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, YRES+MENUSIZE)),
	toolTip(""),
	toolTipPresence(0),
	ren(NULL)
{
	ui::Checkbox * tCheckbox;

	tCheckbox = new ui::Checkbox(ui::Point(1, YRES+4), ui::Point(55, 16), "Effects", "");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_EFFE));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(1, YRES+4+18), ui::Point(55, 16), "Fire", "");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_FIRE));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(61, YRES+4), ui::Point(55, 16), "Glow", "");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_GLOW));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(61, YRES+4+18), ui::Point(55, 16), "Blur", "");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BLUR));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(121, YRES+4), ui::Point(55, 16), "Blob", "");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BLOB));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(121, YRES+4+18), ui::Point(55, 16), "Point", "");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BASC));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(186, YRES+4), ui::Point(70, 16), "Alt. Air", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRC));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(186, YRES+4+18), ui::Point(70, 16), "Pressure", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRP));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(261, YRES+4), ui::Point(70, 16), "Velocity", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRV));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(261, YRES+4+18), ui::Point(70, 16), "Air-heat", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIRH));
	AddComponent(tCheckbox);

	/*tCheckbox = new ui::Checkbox(ui::Point(336, YRES+4), ui::Point(70, 16), "Air", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_AIR));
	AddComponent(tCheckbox);*/

	tCheckbox = new ui::Checkbox(ui::Point(336, YRES+4+18), ui::Point(70, 16), "Warp", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_WARP));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(411, YRES+4), ui::Point(70, 16), "Persistent", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_PERS));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(336, YRES+4), ui::Point(70, 16), "Effect", "");
	displayModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new DisplayModeAction(this, DISPLAY_EFFE));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(486, YRES+4), ui::Point(50, 16), "Heat", "");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_HEAT));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(486, YRES+4+18), ui::Point(50, 16), "Life", "");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_LIFE));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(536, YRES+4+18), ui::Point(50, 16), "H-Gradient", "");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_GRAD));
	AddComponent(tCheckbox);

	tCheckbox = new ui::Checkbox(ui::Point(536, YRES+4), ui::Point(50, 16), "Basic", "");
	colourModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new ColourModeAction(this, COLOUR_BASC));
	AddComponent(tCheckbox);
}

void RenderView::OnMouseDown(int x, int y, unsigned button)
{
	if(x > XRES || y < YRES)
		c->Exit();
}

void RenderView::NotifyRendererChanged(RenderModel * sender)
{
	ren = sender->GetRenderer();
}

void RenderView::NotifyRenderChanged(RenderModel * sender)
{
	for(int i = 0; i < renderModes.size(); i++)
	{
		if(renderModes[i]->GetActionCallback())
		{
			//Compares bitmasks at the moment, this means that "Point" is always on when other options that depend on it are, this might confuse some users, TODO: get the full list and compare that?
			RenderModeAction * action = (RenderModeAction *)(renderModes[i]->GetActionCallback());
			if(action->renderMode  == (sender->GetRenderMode() & action->renderMode))
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
	for(int i = 0; i < displayModes.size(); i++)
	{
		if(displayModes[i]->GetActionCallback())
		{
			DisplayModeAction * action = (DisplayModeAction *)(displayModes[i]->GetActionCallback());
			if(action->displayMode  == (sender->GetDisplayMode() & action->displayMode))
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
	for(int i = 0; i < colourModes.size(); i++)
	{
		if(colourModes[i]->GetActionCallback())
		{
			ColourModeAction * action = (ColourModeAction *)(colourModes[i]->GetActionCallback());
			if(action->colourMode == sender->GetColourMode())
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
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(-1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1);
	if(ren)
	{
		ren->clearScreen(1.0f);
		ren->RenderBegin();
		ren->RenderEnd();
	}
	g->draw_line(0, YRES, XRES-1, YRES, 200, 200, 200, 255);
	g->draw_line(180, YRES, 180, YRES+MENUSIZE, 200, 200, 200, 255);
	g->draw_line(330, YRES, 330, YRES+MENUSIZE, 200, 200, 200, 255);
	g->draw_line(480, YRES, 480, YRES+MENUSIZE, 200, 200, 200, 255);
	g->draw_line(XRES, 0, XRES, YRES+MENUSIZE, 255, 255, 255, 255);
	if(toolTipPresence && toolTip.length())
	{
		g->drawtext(6, Size.Y-MENUSIZE-10, (char*)toolTip.c_str(), 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
	}
}

void RenderView::OnTick(float dt)
{
	if(toolTipPresence>0)
	{
		toolTipPresence -= int(dt)>0?int(dt):1;
		if(toolTipPresence<0)
			toolTipPresence = 0;
	}
}

void RenderView::ToolTip(ui::Component * sender, ui::Point mousePosition, std::string toolTip)
{
	this->toolTip = toolTip;
	toolTipPresence = 160;
}

RenderView::~RenderView() {
	// TODO Auto-generated destructor stub
}
