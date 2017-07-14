#include <iostream>
#include "gui/interface/Button.h"
#include "gui/interface/Window.h"
#include "graphics/Graphics.h"
#include "Misc.h"

namespace ui {

Button::Button(Point position, Point size, std::string buttonText, std::string toolTip):
	Component(position, size),
	ButtonText(buttonText),
	toolTip(toolTip),
	isButtonDown(false),
	isMouseInside(false),
	isTogglable(false),
	toggle(false),
	actionCallback(NULL)
{
	TextPosition(ButtonText);
}

void Button::TextPosition(std::string ButtonText)
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
	TextPosition(ButtonText);
}

void Button::SetText(std::string buttonText)
{
	ButtonText = buttonText;
	TextPosition(ButtonText);
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

bool Button::GetToggleState()
{
	return toggle;
}

void Button::SetToggleState(bool state)
{
	toggle = state;
}

void Button::Draw(const Point& screenPos)
{
	if(!drawn)
	{
		TextPosition(ButtonText);
		drawn = true;
	}
	Graphics * g = GetGraphics();
	Point Position = screenPos;
	ui::Colour bgColour(0, 0, 0);

	ui::Colour textColour = Appearance.TextInactive;
	ui::Colour borderColour = Appearance.BorderInactive;
	ui::Colour backgroundColour = Appearance.BackgroundInactive;

	if (Enabled)
	{
		if (isButtonDown || (isTogglable && toggle))
		{
			textColour = Appearance.TextActive;
			borderColour = Appearance.BorderActive;
			backgroundColour = Appearance.BackgroundActive;
		}
		else if (isMouseInside)
		{
			textColour = Appearance.TextHover;
			borderColour = Appearance.BorderHover;
			backgroundColour = Appearance.BackgroundHover;
		}
		else
		{
			textColour = Appearance.TextInactive;
			borderColour = Appearance.BorderInactive;
			backgroundColour = Appearance.BackgroundInactive;
		}
	}
	else
	{
		textColour = Appearance.TextDisabled;
		borderColour = Appearance.BorderDisabled;
		backgroundColour = Appearance.BackgroundDisabled;
	}

	bgColour = Appearance.BackgroundInactive;
	g->fillrect(Position.X+1, Position.Y+1, Size.X-2, Size.Y-2, backgroundColour.Red, backgroundColour.Green, backgroundColour.Blue, backgroundColour.Alpha);
	if(Appearance.Border == 1)
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, borderColour.Red, borderColour.Green, borderColour.Blue, borderColour.Alpha);
	else
	{
		if(Appearance.Border.Top)
			g->draw_line(Position.X, Position.Y, Position.X+Size.X-1, Position.Y, borderColour.Red, borderColour.Green, borderColour.Blue, borderColour.Alpha);
		if(Appearance.Border.Bottom)
			g->draw_line(Position.X, Position.Y+Size.Y-1, Position.X+Size.X-1, Position.Y+Size.Y-1, borderColour.Red, borderColour.Green, borderColour.Blue, borderColour.Alpha);
		if(Appearance.Border.Left)
			g->draw_line(Position.X, Position.Y, Position.X, Position.Y+Size.Y-1, borderColour.Red, borderColour.Green, borderColour.Blue, borderColour.Alpha);
		if(Appearance.Border.Right)
			g->draw_line(Position.X+Size.X-1, Position.Y, Position.X+Size.X-1, Position.Y+Size.Y-1, borderColour.Red, borderColour.Green, borderColour.Blue, borderColour.Alpha);
	}
	g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, buttonDisplayText, textColour.Red, textColour.Green, textColour.Blue, textColour.Alpha);

	bool iconInvert = (backgroundColour.Blue + (3*backgroundColour.Green) + (2*backgroundColour.Red))>544?true:false;

	if(Appearance.icon)
	{
		if(Enabled)
			g->draw_icon(Position.X+iconPosition.X, Position.Y+iconPosition.Y, Appearance.icon, 255, iconInvert);
		else
			g->draw_icon(Position.X+iconPosition.X, Position.Y+iconPosition.Y, Appearance.icon, 180, iconInvert);
	}
}

void Button::OnMouseUnclick(int x, int y, unsigned int button)
{
	if(button == 1)
	{
		if(isButtonDown)
		{
			if(isTogglable)
			{
				toggle = !toggle;
			}
			isButtonDown = false;
			DoAction();
		}
	}
	else if(button == 3)
	{
		if(isAltButtonDown)
		{
			isAltButtonDown = false;
			DoAltAction();
		}
	}
}

void Button::OnMouseUp(int x, int y, unsigned int button)
{
	// mouse was unclicked, reset variables in case the unclick happened outside
	isButtonDown = false;
	isAltButtonDown = false;
}

void Button::OnMouseClick(int x, int y, unsigned int button)
{
	if(!Enabled)
		return;
	if(button == 1)
	{
		isButtonDown = true;
	}
	else if(button == 3)
	{
		isAltButtonDown = true;
	}
}

void Button::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
	if(!Enabled)
		return;
	if(actionCallback)
		actionCallback->MouseEnterCallback(this);
}

void Button::OnMouseHover(int x, int y)
{
	if(Enabled && toolTip.length()>0 && GetParentWindow())
	{
		GetParentWindow()->ToolTip(Position, toolTip);
	}
}

void Button::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
	isButtonDown = false;
}

void Button::DoAction()
{
	if(!Enabled)
		return;
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void Button::DoAltAction()
{
	if(!Enabled)
		return;
	if(actionCallback)
		actionCallback->AltActionCallback(this);
}

void Button::SetActionCallback(ButtonAction * action)
{
	delete actionCallback;
	actionCallback = action;
}

Button::~Button()
{
	delete actionCallback;
}

} /* namespace ui */
