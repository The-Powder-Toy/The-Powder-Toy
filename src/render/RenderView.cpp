/*
 * RenderView.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#include "simulation/ElementGraphics.h"
#include "Graphics.h"
#include "Renderer.h"
#include "RenderView.h"

class RenderView::RenderModeAction: public ui::CheckboxAction
{
	RenderView * v;
	unsigned int renderMode;
public:
	RenderModeAction(RenderView * v_, unsigned int renderMode_)
	{
		v = v_;
		renderMode = renderMode_;
	}
	virtual void ActionCallback(ui::Checkbox * sender)
	{
		if(sender->IsChecked())
			v->c->SetRenderMode(renderMode);
		else
			v->c->UnsetRenderMode(renderMode);
	}
};

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, YRES+MENUSIZE)),
	ren(NULL)
{
	ui::Checkbox * tCheckbox;
	tCheckbox = new ui::Checkbox(ui::Point(0, YRES+5), ui::Point(100, 16), "Blob");
	renderModes.push_back(tCheckbox);
	tCheckbox->SetActionCallback(new RenderModeAction(this, RENDER_BLOB));
	AddComponent(tCheckbox);

}

void RenderView::NotifyRendererChanged(RenderModel * sender)
{
	ren = sender->GetRenderer();
}

void RenderView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(0, 0, XRES, YRES+MENUSIZE);
	g->draw_line(XRES-1, 0, XRES-1, YRES+MENUSIZE, 255, 255, 255, XRES+BARSIZE);
	if(ren)
	{
		ren->render_parts();
		ren->render_fire();
	}
}

RenderView::~RenderView() {
	// TODO Auto-generated destructor stub
}
