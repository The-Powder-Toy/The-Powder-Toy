/*
 * Checkbox.cpp
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#include "Checkbox.h"

using namespace ui;

Checkbox::Checkbox(ui::Point position, ui::Point size, std::string text):
	Component(position, size),
	text(text),
	isMouseOver(false),
	checked(false),
	actionCallback(NULL)
{
	// TODO Auto-generated constructor stub

}

void Checkbox::SetText(std::string text)
{
	this->text = text;
}

void Checkbox::OnMouseClick(int x, int y, unsigned int button)
{
	if(checked)
	{
		checked = false;
	}
	else
	{
		checked = true;
	}
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void Checkbox::OnMouseUp(int x, int y, unsigned int button)
{

}


void Checkbox::OnMouseEnter(int x, int y)
{
	isMouseOver = true;
}

void Checkbox::OnMouseLeave(int x, int y)
{
	isMouseOver = false;
}

void Checkbox::Draw(const Point& screenPos)
{
	Graphics * g = Engine::Ref().g;
	if(checked)
	{
		g->fillrect(screenPos.X+5, screenPos.Y+5, 6, 6, 255, 255, 255, 255);
	}
	if(isMouseOver)
	{
		g->drawrect(screenPos.X+2, screenPos.Y+2, 12, 12, 255, 255, 255, 255);
		g->fillrect(screenPos.X+5, screenPos.Y+5, 6, 6, 255, 255, 255, 170);
		g->drawtext(screenPos.X+18, screenPos.Y+4, text, 255, 255, 255, 255);
	}
	else
	{
		g->drawrect(screenPos.X+2, screenPos.Y+2, 12, 12, 255, 255, 255, 200);
		g->drawtext(screenPos.X+18, screenPos.Y+4, text, 255, 255, 255, 200);
	}
}

void Checkbox::SetActionCallback(CheckboxAction * action)
{
	if(actionCallback)
		delete actionCallback;
	actionCallback = action;
}

Checkbox::~Checkbox() {
	if(actionCallback)
		delete actionCallback;
}

