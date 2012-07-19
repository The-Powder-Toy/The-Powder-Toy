/*
 * Button.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <iostream>
#include "interface/Button.h"
#include "graphics/Graphics.h"
#include "Engine.h"
#include "Misc.h"

namespace ui {

Button::Button(Point position, Point size, std::string buttonText, std::string toolTip):
	Component(position, size),
	ButtonText(buttonText),
	isMouseInside(false),
	isButtonDown(false),
	isTogglable(false),
	toggle(false),
	actionCallback(NULL),
	Enabled(true),
	toolTip(toolTip)
{
	TextPosition();
}

void Button::TextPosition()
{
	buttonDisplayText = ButtonText;
	if(buttonDisplayText.length())
	{
		if(Graphics::textwidth((char *)buttonDisplayText.c_str()) > Size.X - (Appearance.icon? 22 : 0))
		{
			int position = Graphics::textwidthx((char *)buttonDisplayText.c_str(), Size.X - (Appearance.icon? 38 : 22));
			buttonDisplayText = buttonDisplayText.erase(position, buttonDisplayText.length()-position);
			buttonDisplayText += "...";
		}
	}

	Component::TextPosition(buttonDisplayText);
}

void Button::SetIcon(Icon icon)
{
	Appearance.icon = icon;
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
	if(!drawn)
	{
		TextPosition();
		drawn = true;
	}
	Graphics * g = ui::Engine::Ref().g;
	Point Position = screenPos;
	if(Enabled)
	{
		if(isButtonDown || (isTogglable && toggle))
		{
			g->fillrect(Position.X+1, Position.Y+1, Size.X-2, Size.Y-2, Appearance.BackgroundActive.Red, Appearance.BackgroundActive.Green, Appearance.BackgroundActive.Blue, 255);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BorderActive.Red, Appearance.BorderActive.Green, Appearance.BorderActive.Blue, 255);
			g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, buttonDisplayText, Appearance.TextActive.Red, Appearance.TextActive.Green, Appearance.TextActive.Blue, 255);
		}
		else
		{
			g->fillrect(Position.X+1, Position.Y+1, Size.X-2, Size.Y-2, Appearance.BackgroundInactive.Red, Appearance.BackgroundInactive.Green, Appearance.BackgroundInactive.Blue, 255);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BorderInactive.Red, Appearance.BorderInactive.Green, Appearance.BorderInactive.Blue, 255);
			g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, buttonDisplayText, Appearance.TextInactive.Red, Appearance.TextInactive.Green, Appearance.TextInactive.Blue, 255);
		}
	}
	else
	{
		g->fillrect(Position.X+1, Position.Y+1, Size.X-2, Size.Y-2, Appearance.BackgroundInactive.Red, Appearance.BackgroundInactive.Green, Appearance.BackgroundInactive.Blue, 180);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BackgroundDisabled.Red, Appearance.BackgroundDisabled.Green, Appearance.BackgroundDisabled.Blue, Appearance.BackgroundDisabled.Alpha);
		g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, buttonDisplayText, 180, 180, 180, 255);
	}
	if(Appearance.icon)
		g->draw_icon(Position.X+iconPosition.X, Position.Y+iconPosition.Y, Appearance.icon);
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
	if(!Enabled)
		return;
	if(actionCallback)
		actionCallback->MouseEnterCallback(this);
	if(toolTip.length()>0 && GetParentWindow())
	{
		GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip);
	}
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
