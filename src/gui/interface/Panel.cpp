#include "gui/interface/Panel.h"

#include "gui/interface/Engine.h"
#include "gui/interface/Window.h"

#include "graphics/Graphics.h"

using namespace ui;

Panel::Panel(Point position, Point size):
	Component(position, size),
	InnerSize(size),
	ViewportPosition(0, 0),
	mouseInside(false)
{
}

Panel::~Panel()
{
	for(unsigned i = 0; i < children.size(); ++i)
	{
		delete children[i];
	}
}

void Panel::AddChild(Component* c)
{
	c->SetParent(this);
	c->SetParentWindow(this->GetParentWindow());
}

int Panel::GetChildCount()
{
	return children.size();
}

Component* Panel::GetChild(unsigned idx)
{
	return children[idx];
}

void Panel::RemoveChild(Component* c)
{
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i] == c)
		{
			//remove child from parent. Does not free memory
			children.erase(children.begin() + i);
			if (this->GetParentWindow()->IsFocused(c))
				this->GetParentWindow()->FocusComponent(NULL);
			break;
		}
	}
}

void Panel::RemoveChild(unsigned idx, bool freeMem)
{
	if(freeMem)
		delete children[idx];

	children.erase(children.begin() + idx);
}

void Panel::Draw(const Point& screenPos)
{
	// draw ourself first
	XDraw(screenPos);

	int x = screenPos.X;
	int y = screenPos.Y;
	int w = Size.X;
	int h = Size.Y;
	ui::Engine::Ref().g->SetClipRect(x, y, w, h); // old cliprect is now in x, y, w, h

	// attempt to draw all children
	for (size_t i = 0; i < children.size(); ++i)
	{
		// the component must be visible
		if (children[i]->Visible)
		{
			//check if the component is in the screen, draw if it is
			if (children[i]->Position.X + ViewportPosition.X + children[i]->Size.X >= 0 &&
				children[i]->Position.Y + ViewportPosition.Y + children[i]->Size.Y >= 0 &&
				children[i]->Position.X + ViewportPosition.X < ui::Engine::Ref().GetWidth() &&
				children[i]->Position.Y + ViewportPosition.Y < ui::Engine::Ref().GetHeight() )
			{
				Point scrpos = screenPos + children[i]->Position + ViewportPosition;
				children[i]->Draw(scrpos);
			}
		}
	}

	ui::Engine::Ref().g->SetClipRect(x, y, w, h); // apply old cliprect
}

void Panel::Tick(float dt)
{
	// tick ourself first
	XTick(dt);

	// tick our children
	for(unsigned i = 0; i < children.size(); ++i)
		children[i]->Tick(dt);
}

void Panel::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	XOnKeyPress(key, scan, repeat, shift, ctrl, alt);
}

void Panel::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	XOnKeyRelease(key, scan, repeat, shift, ctrl, alt);
}

void Panel::OnMouseClick(int localx, int localy, unsigned button)
{
	bool childclicked = false;

	//check if clicked a child
	for(int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be enabled
		if(children[i]->Enabled)
		{
			//is mouse inside?
			if( localx >= children[i]->Position.X + ViewportPosition.X &&
				localy >= children[i]->Position.Y + ViewportPosition.Y &&
				localx < children[i]->Position.X + ViewportPosition.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + ViewportPosition.Y + children[i]->Size.Y )
			{
				childclicked = true;
				GetParentWindow()->FocusComponent(children[i]);
				children[i]->OnMouseClick(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, button);
				break;
			}
		}
	}

	//if a child wasn't clicked, send click to ourself
	if(!childclicked)
	{
		XOnMouseClick(localx, localy, button);
		GetParentWindow()->FocusComponent(this);
	}
}

void Panel::OnMouseDown(int x, int y, unsigned button)
{
	XOnMouseDown(x, y, button);
	for (size_t i = 0; i < children.size(); ++i)
	{
		if(children[i]->Enabled)
			children[i]->OnMouseDown(x, y, button);
	}
}

void Panel::OnMouseHover(int localx, int localy)
{
	// check if hovering on children
	for (int i = children.size() - 1; i >= 0; --i)
	{
		if (children[i]->Enabled)
		{
			if( localx >= children[i]->Position.X &&
				localy >= children[i]->Position.Y &&
				localx < children[i]->Position.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + children[i]->Size.Y )
			{
				children[i]->OnMouseHover(localx - children[i]->Position.X, localy - children[i]->Position.Y);
				break;
			}
		}
	}

	// always allow hover on parent (?)
	XOnMouseHover(localx, localy);
}

void Panel::OnMouseMoved(int localx, int localy, int dx, int dy)
{
	XOnMouseMoved(localx, localy, dx, dy);
	for (size_t i = 0; i < children.size(); ++i)
	{
		if(children[i]->Enabled)
			children[i]->OnMouseMoved(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, dx, dy);
	}
}

void Panel::OnMouseMovedInside(int localx, int localy, int dx, int dy)
{
	mouseInside = true;
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i]->Enabled)
		{
			Point local	(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y)
			, prevlocal (local.X - dx, local.Y - dy);

			// mouse currently inside?
			if( local.X >= 0 &&
				local.Y >= 0 &&
				local.X < children[i]->Size.X &&
				local.Y < children[i]->Size.Y )
			{
				children[i]->OnMouseMovedInside(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, dx, dy);

				// was the mouse outside?
				if(!(prevlocal.X >= 0 &&
					 prevlocal.Y >= 0 &&
					 prevlocal.X < children[i]->Size.X &&
					 prevlocal.Y < children[i]->Size.Y ) )
				{
					children[i]->OnMouseEnter(local.X, local.Y);
				}
			}
			// if not currently inside
			else
			{
				// was the mouse inside?
				if(	prevlocal.X >= 0 &&
					prevlocal.Y >= 0 &&
					prevlocal.X < children[i]->Size.X &&
					prevlocal.Y < children[i]->Size.Y )
				{
					children[i]->OnMouseLeave(local.X, local.Y);
				}

			}
		}
	}

	// always allow hover on parent (?)
	XOnMouseMovedInside(localx, localy, dx, dy);
}

void Panel::OnMouseEnter(int localx, int localy)
{
	mouseInside = true;
	XOnMouseEnter(localx, localy);
}

void Panel::OnMouseLeave(int localx, int localy)
{
	mouseInside = false;
	XOnMouseLeave(localx, localy);
}

void Panel::OnMouseUnclick(int localx, int localy, unsigned button)
{
	bool childunclicked = false;

	//check if clicked a child
	for(int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be unlocked
		if(children[i]->Enabled)
		{
			//is mouse inside?
			if( localx >= children[i]->Position.X + ViewportPosition.X &&
				localy >= children[i]->Position.Y + ViewportPosition.Y &&
				localx < children[i]->Position.X + ViewportPosition.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + ViewportPosition.Y + children[i]->Size.Y )
			{
				childunclicked = true;
				children[i]->OnMouseUnclick(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, button);
				break;
			}
		}
	}

	//if a child wasn't clicked, send click to ourself
	if (!childunclicked)
	{
		XOnMouseUnclick(localx, localy, button);
	}
}

void Panel::OnMouseUp(int x, int y, unsigned button)
{
	XOnMouseUp(x, y, button);
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i]->Enabled)
			children[i]->OnMouseUp(x, y, button);
	}
}

void Panel::OnMouseWheel(int localx, int localy, int d)
{
	XOnMouseWheel(localx, localy, d);
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i]->Enabled)
			children[i]->OnMouseWheel(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, d);
	}
}

void Panel::OnMouseWheelInside(int localx, int localy, int d)
{
	XOnMouseWheelInside(localx, localy, d);
	//check if clicked a child
	for (int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be unlocked
		if (children[i]->Enabled)
		{
			//is mouse inside?
			if (localx >= children[i]->Position.X + ViewportPosition.X &&
				localy >= children[i]->Position.Y + ViewportPosition.Y &&
				localx < children[i]->Position.X + ViewportPosition.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + ViewportPosition.Y + children[i]->Size.Y )
			{
				children[i]->OnMouseWheelInside(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, d);
				break;
			}
		}
	}
}

// ***** OVERRIDEABLES *****
// Kept empty.

void Panel::XDraw(const Point& screenPos)
{
}

void Panel::XTick(float dt)
{
}

void Panel::XOnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
}

void Panel::XOnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
}

void Panel::XOnMouseClick(int localx, int localy, unsigned button)
{
}

void Panel::XOnMouseDown(int x, int y, unsigned button)
{
}

void Panel::XOnMouseHover(int localx, int localy)
{
}

void Panel::XOnMouseMoved(int localx, int localy, int dx, int dy)
{
}

void Panel::XOnMouseMovedInside(int localx, int localy, int dx, int dy)
{
}

void Panel::XOnMouseEnter(int localx, int localy)
{
}

void Panel::XOnMouseLeave(int localx, int localy)
{
}

void Panel::XOnMouseUnclick(int localx, int localy, unsigned button)
{
}

void Panel::XOnMouseUp(int x, int y, unsigned button)
{
}

void Panel::XOnMouseWheel(int localx, int localy, int d)
{
}

void Panel::XOnMouseWheelInside(int localx, int localy, int d)
{
}
