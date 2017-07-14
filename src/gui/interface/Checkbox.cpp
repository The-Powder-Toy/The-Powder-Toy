#include "Checkbox.h"
#include "graphics/Graphics.h"
#include "gui/interface/Window.h"

using namespace ui;

Checkbox::Checkbox(ui::Point position, ui::Point size, std::string text, std::string toolTip):
	Component(position, size),
	text(text),
	toolTip(toolTip),
	checked(false),
	isMouseOver(false),
	actionCallback(NULL)
{

}

void Checkbox::SetText(std::string text)
{
	this->text = text;
}

std::string Checkbox::GetText()
{
	return text;
}

void Checkbox::SetIcon(Icon icon)
{
	Appearance.icon = icon;
	iconPosition.X = 16;
	iconPosition.Y = 3;
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

void Checkbox::OnMouseHover(int x, int y)
{
	if(toolTip.length()>0 && GetParentWindow())
	{
		GetParentWindow()->ToolTip(Position, toolTip);
	}
}

void Checkbox::OnMouseLeave(int x, int y)
{
	isMouseOver = false;
}

void Checkbox::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	if(checked)
	{
		g->fillrect(screenPos.X+5, screenPos.Y+5, 6, 6, 255, 255, 255, 255);
	}
	if(isMouseOver)
	{
		g->drawrect(screenPos.X+2, screenPos.Y+2, 12, 12, 255, 255, 255, 255);
		g->fillrect(screenPos.X+5, screenPos.Y+5, 6, 6, 255, 255, 255, 170);
		if (!Appearance.icon)
			g->drawtext(screenPos.X+18, screenPos.Y+4, text, 255, 255, 255, 255);
		else
			g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon, 255);
	}
	else
	{
		g->drawrect(screenPos.X+2, screenPos.Y+2, 12, 12, 255, 255, 255, 200);
		if (!Appearance.icon)
			g->drawtext(screenPos.X+18, screenPos.Y+4, text, 255, 255, 255, 200);
		else
			g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon, 200);
	}
}

void Checkbox::SetActionCallback(CheckboxAction * action)
{
	delete actionCallback;
	actionCallback = action;
}

Checkbox::~Checkbox() {
	delete actionCallback;
}

