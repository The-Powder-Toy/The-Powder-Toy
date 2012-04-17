/*
 * DropDown.cpp
 *
 *  Created on: Apr 16, 2012
 *      Author: Simon
 */

#include "DropDown.h"

namespace ui {

class DropDownWindow: public ui::Window {
	Colour background, activeBackground;
	Colour border, activeBorder;
	Colour text, activeText;
	bool isMouseInside;
public:
	DropDownWindow(Point position, Point size, Colour background, Colour activeBackground, Colour border, Colour activeBorder, Colour text, Colour activeText):
		Window(position, size),
		background(background),
		activeBackground(activeBackground),
		border(border),
		activeBorder(activeBorder),
		text(text),
		activeText(activeText)
	{

	}
	virtual void OnDraw()
	{
		Graphics * g = ui::Engine::Ref().g;
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, border.Red, border.Green, border.Blue, 255);
	}
	virtual ~DropDownWindow() {}
};

DropDown::DropDown(Point position, Point size):
	Component(position, size),
	isMouseInside(false)
{
	activeText = background = Colour(0, 0, 0);
	text = activeBackground = border = activeBorder = Colour(255, 255, 255);
}

void DropDown::OnMouseClick(int x, int y, unsigned int button)
{
	ui::Engine().Ref().ShowWindow(new DropDownWindow(ui::Point(50, 50), ui::Point(50, 50), background, activeBackground, border, activeBorder, text, activeText));
}

void DropDown::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	Point Position = screenPos;
	if(isMouseInside)
	{
		g->fillrect(Position.X-1, Position.Y-1, Size.X+2, Size.Y+2, activeBackground.Red, activeBackground.Green, activeBackground.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, activeBorder.Red, activeBorder.Green, activeBorder.Blue, 255);
		//g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y+1, displayText, activeText.Red, activeText.Green, activeText.Blue, 255);
	}
	else
	{
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, border.Red, border.Green, border.Blue, 255);
		//g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y+1, displayText, text.Red, text.Green, text.Blue, 255);
	}

}


DropDown::~DropDown() {
	// TODO Auto-generated destructor stub
}

} /* namespace ui */
