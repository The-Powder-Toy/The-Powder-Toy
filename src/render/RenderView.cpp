/*
 * RenderView.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#include "RenderView.h"
#include "Graphics.h"

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, YRES+MENUSIZE)),
	ren(NULL)
{
	ui::Checkbox * tCheckbox;
	tCheckbox = new ui::Checkbox(ui::Point(0, YRES+5), ui::Point(100, 16), "Thing");
	renderModes.push_back(tCheckbox);
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
