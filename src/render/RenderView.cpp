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
	// TODO Auto-generated constructor stub

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
