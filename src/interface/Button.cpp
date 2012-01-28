/*
 * Button.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <iostream>

#include "interface/Button.h"
#include "Graphics.h"
#include "Global.h"
#include "Engine.h"
#include "Misc.h"

namespace ui {

Button::Button(Point position, Point size, std::string buttonText):
	Component(position, size),
	ButtonText(buttonText),
	isMouseInside(false),
	isButtonDown(false),
	isTogglable(false),
	toggle(false),
	actionCallback(NULL),
	textPosition(ui::Point(0, 0)),
	textVAlign(AlignMiddle),
	textHAlign(AlignCentre),
	Enabled(true),
	icon(NoIcon)
{
	activeText = background = Colour(0, 0, 0);
	text = activeBackground = border = activeBorder = Colour(255, 255, 255);
	TextPosition();
}

void Button::TextPosition()
{
	switch(textVAlign)
	{
	case AlignTop:
		textPosition.Y = 3;
		break;
	case AlignMiddle:
		textPosition.Y = (Size.Y-10)/2;
		break;
	case AlignBottom:
		textPosition.Y = Size.Y-11;
		break;
	}

	if(icon)
	{
		switch(textHAlign)
		{
		case AlignLeft:
			textPosition.X = 3+17;
			break;
		case AlignCentre:
			textPosition.X = (((Size.X-14)-Graphics::textwidth((char *)ButtonText.c_str()))/2)+17;
			break;
		case AlignRight:
			textPosition.X = (((Size.X-14)-Graphics::textwidth((char *)ButtonText.c_str()))-2)+17;
			break;
		}
	}
	else
	{
		switch(textHAlign)
		{
		case AlignLeft:
			textPosition.X = 3;
			break;
		case AlignCentre:
			textPosition.X = (Size.X-Graphics::textwidth((char *)ButtonText.c_str()))/2;
			break;
		case AlignRight:
			textPosition.X = (Size.X-Graphics::textwidth((char *)ButtonText.c_str()))-2;
			break;
		}
	}
}

void Button::SetIcon(Icon icon)
{
	this->icon = icon;
	TextPosition();
}

void Button::SetText(std::string buttonText)
{
	ButtonText = buttonText;
	TextPosition();
}

void Button::SetTogglable(bool togglable)
{
	toggle = false;
	isTogglable = togglable;
}

bool Button::GetTogglable()
{
	return isTogglable;
}

inline bool Button::GetToggleState()
{
	return toggle;
}

inline void Button::SetToggleState(bool state)
{
	toggle = state;
}

void Button::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	Point Position = screenPos;
	if(Enabled)
	{
		if(isButtonDown || (isTogglable && toggle))
		{
			g->fillrect(Position.X-1, Position.Y-1, Size.X+2, Size.Y+2, activeBackground.Red, activeBackground.Green, activeBackground.Blue, 255);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, activeBorder.Red, activeBorder.Green, activeBorder.Blue, 255);
			g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, ButtonText, activeText.Red, activeText.Green, activeText.Blue, 255);
		}
		else
		{
			g->fillrect(Position.X, Position.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 255);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, border.Red, border.Green, border.Blue, 255);
			g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, ButtonText, text.Red, text.Green, text.Blue, 255);
		}
	}
	else
	{
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 180);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 180, 180, 180, 255);
		g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, ButtonText, 180, 180, 180, 255);
	}
	if(icon)
		g->draw_icon(Position.X+3, Position.Y+textPosition.Y, icon);
}

void Button::OnMouseUp(int x, int y, unsigned int button)
{
    if(button != 1)
    {
        return;
    }

    if(isButtonDown)
    {
		DoAction();
    }

    isButtonDown = false;
}

void Button::OnMouseClick(int x, int y, unsigned int button)
{
    if(button != 1) return;
	if(isTogglable)
	{
		toggle = !toggle;
	}
    isButtonDown = true;
}

void Button::OnMouseEnter(int x, int y)
{
    isMouseInside = true;
}

void Button::OnMouseLeave(int x, int y)
{
    isMouseInside = false;
}

void Button::DoAction()
{
	if(!Enabled)
		return;
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void Button::SetActionCallback(ButtonAction * action)
{
	if(actionCallback)
		delete actionCallback;
	actionCallback = action;
}

Button::~Button()
{
	if(actionCallback)
		delete actionCallback;
}

} /* namespace ui */
