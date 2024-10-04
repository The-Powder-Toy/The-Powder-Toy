#include "gui/interface/Button.h"

#include <utility>

#include "gui/interface/Window.h"

#include "graphics/Graphics.h"
#include "Misc.h"
#include "Colour.h"

namespace ui {

Button::Button(Point position, Point size, String buttonText, String toolTip):
	Component(position, size),
	ButtonText(std::move(buttonText)),
	toolTip(std::move(toolTip)),
	isButtonDown(false),
	isMouseInside(false),
	isTogglable(false),
	toggle(false)
{
	TextPosition(ButtonText);
}

void Button::TextPosition(const String& ButtonText)
{
	buttonDisplayText = ButtonText;
	if(buttonDisplayText.length())
	{
		if (Graphics::TextSize(buttonDisplayText).X - 1 > Size.X - (Appearance.icon ? 22 : 0))
		{
			auto it = Graphics::TextFit(buttonDisplayText, Size.X - (Appearance.icon ? 38 : 22));
			buttonDisplayText.erase(it, buttonDisplayText.end());
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

void Button::SetText(String buttonText)
{
	ButtonText = std::move(buttonText);
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
		if ((isButtonDown && MouseDownInside) || (isTogglable && toggle))
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
	g->BlendFilledRect(RectSized(Position + Vec2{ 1, 1 }, Size - Vec2{ 2, 2 }), backgroundColour);
	if(Appearance.Border == 1)
		g->BlendRect(RectSized(Position, Size), borderColour);
	else
	{
		if(Appearance.Border.Top)
			g->BlendLine(Position + Vec2{       0 ,        0 }, Position + Vec2{ Size.X-1,        0 }, borderColour);
		if(Appearance.Border.Bottom)
			g->BlendLine(Position + Vec2{       0 , Size.Y-1 }, Position + Vec2{ Size.X-1, Size.Y-1 }, borderColour);
		if(Appearance.Border.Left)
			g->BlendLine(Position + Vec2{       0 ,        0 }, Position + Vec2{        0, Size.Y-1 }, borderColour);
		if(Appearance.Border.Right)
			g->BlendLine(Position + Vec2{ Size.X-1,        0 }, Position + Vec2{ Size.X-1, Size.Y-1 }, borderColour);
	}
	g->BlendText(Position + textPosition, buttonDisplayText, textColour);

	bool iconInvert = (backgroundColour.Blue + (3*backgroundColour.Green) + (2*backgroundColour.Red))>544?true:false;

	if(Appearance.icon)
	{
		if(Enabled)
			g->draw_icon(Position.X+iconPosition.X, Position.Y+iconPosition.Y, Appearance.icon, 255, iconInvert);
		else
			g->draw_icon(Position.X+iconPosition.X, Position.Y+iconPosition.Y, Appearance.icon, 180, iconInvert);
	}
}

void Button::OnMouseClick(int x, int y, unsigned int button)
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

void Button::OnMouseDown(int x, int y, unsigned int button)
{
	if (MouseDownInside)
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
}

void Button::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
	if(!Enabled)
		return;
	if (actionCallback.mouseEnter)
		actionCallback.mouseEnter();
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
	if (actionCallback.action)
		actionCallback.action();
}

void Button::DoAltAction()
{
	if(!Enabled)
		return;
	if (actionCallback.altAction)
		actionCallback.altAction();
}

} /* namespace ui */
