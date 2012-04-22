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
	activeBorder = ui::Colour(255, 0, 0);
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
	int totalColour = background.Red + (3*background.Green) + (2*background.Blue);

	g->fillrect(screenPos.X+2, screenPos.Y+2, Size.X-4, Size.Y-4, background.Red, background.Green, background.Blue, background.Alpha);

	if(isMouseInside && currentSelection == -1)
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, activeBorder.Red, activeBorder.Green, activeBorder.Blue, activeBorder.Alpha);
	}
	else
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, border.Red, border.Green, border.Blue, border.Alpha);
	}

	if (totalColour<544)
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText.c_str(), 255, 255, 255, 255);
	}
	else
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText.c_str(), 0, 0, 0, 255);
	}
}

void ToolButton::SetSelectionState(int state)
{
	currentSelection = state;
	switch(state)
	{
	case 0:
		border = ui::Colour(255, 0, 0);
		break;
	case 1:
		border = ui::Colour(0, 0, 255);
		break;
	case 2:
		border = ui::Colour(0, 255, 0);
		break;
	default:
		border = ui::Colour(0, 0, 0);
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

