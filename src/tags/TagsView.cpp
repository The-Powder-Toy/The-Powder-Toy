/*
 * TagsView.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#include "TagsView.h"

#include "TagsController.h"
#include "TagsModel.h"

TagsView::TagsView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 300)){
	// TODO Auto-generated constructor stub

}

void TagsView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

TagsView::~TagsView() {
	// TODO Auto-generated destructor stub
}

