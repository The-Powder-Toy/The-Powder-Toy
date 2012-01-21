/*
 * PreviewView.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include "PreviewView.h"
#include "interface/Point.h"
#include "interface/Window.h"

PreviewView::PreviewView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 200))
{
	// TODO Auto-generated constructor stub

}

void PreviewView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

PreviewView::~PreviewView() {
	// TODO Auto-generated destructor stub
}

