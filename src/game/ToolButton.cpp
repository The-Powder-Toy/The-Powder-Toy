/*
 * ToolButton.cpp
 *
 *  Created on: Jan 30, 2012
 *      Author: Simon
 */

#include "ToolButton.h"
#include "interface/Keys.h"

ToolButton::ToolButton(ui::Point position, ui::Point size, std::string text_):
	ui::Button(position, size, text_)
{
	SetSelectionState(-1);
}

void ToolButton::OnMouseClick(int x, int y, unsigned int button)
{
    isButtonDown = true;
}

void ToolButton::OnMouseUp(int x, int y, unsigned int button)
{
	if(isButtonDown)
	{
		if(button == BUTTON_LEFT)
			SetSelectionState(0);
		if(button == BUTTON_RIGHT)
			SetSelectionState(1);
		if(button == BUTTON_MIDDLE)
			SetSelectionState(2);
		DoAction();
	}
	isButtonDown = false;
}

void ToolButton::Draw(const ui::Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	int totalColour = background.Red + 3*background.Green + 2*background.Blue;

	g->fillrect(screenPos.X, screenPos.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 255);

	if (totalColour<544)
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText.c_str(), 255, 255, 255, 255);
	}
	else
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText.c_str(), 0, 0, 0, 255);
	}
	if(currentSelection!=-1)
	{
		//g->fillrect(screenPos.X+1, screenPos.Y+1, Size.X-2, Size.Y-2, 255, 255, 255, 170);
		g->fillrect(screenPos.X+2, screenPos.Y+2, Size.Y-4, Size.Y-4, 0, 0, 0, 170);
		g->drawtext(screenPos.X+5, screenPos.Y+4, selectionText, 255, 255, 255, 255);
	}
}

void ToolButton::SetSelectionState(int state)
{
	currentSelection = state;
	switch(state)
	{
	case 0:
		selectionText = "L";
		break;
	case 1:
		selectionText = "R";
		break;
	case 2:
		selectionText = "M";
		break;
	default:
		selectionText = "";
		break;
	}
}

int ToolButton::GetSelectionState()
{
	return currentSelection;
}

ToolButton::~ToolButton() {
	// TODO Auto-generated destructor stub
}

