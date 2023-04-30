#include "Checkbox.h"

#include "graphics/Graphics.h"

#include "gui/interface/Window.h"

using namespace ui;

Checkbox::Checkbox(ui::Point position, ui::Point size, String text, String toolTip):
	Component(position, size),
	text(text),
	toolTip(toolTip),
	checked(false),
	isMouseOver(false)
{

}

void Checkbox::SetText(String text)
{
	this->text = text;
}

String Checkbox::GetText()
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
	if (actionCallback.action)
		actionCallback.action();
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
		g->DrawFilledRect(RectSized(screenPos + Vec2{ 5, 5 }, Vec2{ 6, 6 }), 0xFFFFFF_rgb);
	}
	if(isMouseOver)
	{
		g->DrawRect(RectSized(screenPos + Vec2{ 2, 2 }, Vec2{ 12, 12 }), 0xFFFFFF_rgb);
		g->BlendFilledRect(RectSized(screenPos + Vec2{ 5, 5 }, Vec2{ 6, 6 }), 0xFFFFFF_rgb .WithAlpha(170));
		if (!Appearance.icon)
			g->BlendText(screenPos + Vec2{ 18, 4 }, text, 0xFFFFFF_rgb .WithAlpha(255));
		else
			g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon, 255);
	}
	else
	{
		g->BlendRect(RectSized(screenPos + Vec2{ 2, 2 }, Vec2{ 12, 12 }), 0xFFFFFF_rgb .WithAlpha(200));
		if (!Appearance.icon)
			g->BlendText(screenPos + Vec2{ 18, 4 }, text, 0xFFFFFF_rgb .WithAlpha(200));
		else
			g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon, 200);
	}
}
