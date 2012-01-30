#pragma once
#include <vector>
//#include "Platform.h"

#include "interface/Panel.h"

#include "interface/Point.h"
#include "interface/Window.h"
#include "interface/Component.h"

using namespace ui;

Panel::Panel(Window* parent_state):
	Component(parent_state)
{

}

Panel::Panel(Point position, Point size):
	Component(position, size)
{

}

Panel::Panel():
	Component()
{

}

Panel::~Panel()
{
	for(unsigned i = 0; i < children.size(); ++i)
	{
		if( children[i] )
			delete children[i];
	}
}

void Panel::AddChild(Component* c)
{
	c->SetParent(this);
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
	for(int i = 0; i < children.size(); ++i)
	{
		if(children[i] == c)
		{
			//remove child from parent. Does not free memory
			children.erase(children.begin() + i);
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
	
	// attempt to draw all children
	for(int i = 0; i < children.size(); ++i)
	{
		// the component must be visible
		if(children[i]->Visible)
		{
			if(GetParentWindow()->AllowExclusiveDrawing)
			{
				//who cares if the component is off the screen? draw anyway.
				Point scrpos = screenPos + children[i]->Position;
				children[i]->Draw(scrpos);
			}
			else
			{
				//check if the component is in the screen, draw if it is
				if( children[i]->Position.X + children[i]->Size.X >= 0 &&
					children[i]->Position.Y + children[i]->Size.Y >= 0 &&
					children[i]->Position.X < ui::Engine::Ref().GetWidth() &&
					children[i]->Position.Y < ui::Engine::Ref().GetHeight() )
				{
					Point scrpos = screenPos + children[i]->Position;
					children[i]->Draw(scrpos);
				}
			}
		}
	}
}

void Panel::Tick(float dt)
{
	// tick ourself first
	XTick(dt);
	
	// tick our children
	for(unsigned i = 0; i < children.size(); ++i)
		children[i]->Tick(dt);
}

void Panel::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	XOnKeyPress(key, character, shift, ctrl, alt);
}

void Panel::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	XOnKeyRelease(key, character, shift, ctrl, alt);
}

void Panel::OnMouseClick(int localx, int localy, unsigned button)
{
	bool childclicked = false;
	
	//check if clicked a child
	for(int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be unlocked
		if(!children[i]->Locked)
		{
			//is mouse inside?
			if( localx >= children[i]->Position.X &&
				localy >= children[i]->Position.Y &&
				localx < children[i]->Position.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + children[i]->Size.Y )
			{
				childclicked = true;
				GetParentWindow()->FocusComponent(children[i]);
				children[i]->OnMouseClick(localx - children[i]->Position.X, localy - children[i]->Position.Y, button);
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
	for(int i = 0; i < children.size(); ++i)
	{
		if(!children[i]->Locked)
			children[i]->OnMouseDown(x, y, button);
	}
}

void Panel::OnMouseHover(int localx, int localy)
{
	// check if hovering on children
	for(int i = children.size() - 1; i >= 0; --i)
	{
		if(!children[i]->Locked)
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
	for(int i = 0; i < children.size(); ++i)
	{
		if(!children[i]->Locked)
			children[i]->OnMouseMoved(localx - children[i]->Position.X, localy - children[i]->Position.Y, dx, dy);
	}
}

void Panel::OnMouseMovedInside(int localx, int localy, int dx, int dy)
{
	for(int i = 0; i < children.size(); ++i)
	{
		if(!children[i]->Locked)
		{
			Point local	(localx - children[i]->Position.X, localy - children[i]->Position.Y)
			, prevlocal (local.X - dx, local.Y - dy);
			
			// mouse currently inside?
			if( local.X >= 0 &&
				local.Y >= 0 &&
				local.X < children[i]->Size.X &&
				local.Y < children[i]->Size.Y )
			{
				children[i]->OnMouseMovedInside(localx - children[i]->Position.X, localy - children[i]->Position.Y, dx, dy);
				
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
	XOnMouseEnter(localx, localy);
}

void Panel::OnMouseLeave(int localx, int localy)
{
	XOnMouseLeave(localx, localy);
}

void Panel::OnMouseUnclick(int localx, int localy, unsigned button)
{
	bool childunclicked = false;
	
	//check if clicked a child
	for(int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be unlocked
		if(!children[i]->Locked)
		{
			//is mouse inside?
			if( localx >= children[i]->Position.X &&
				localy >= children[i]->Position.Y &&
				localx < children[i]->Position.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + children[i]->Size.Y )
			{
				childunclicked = true;
				children[i]->OnMouseUnclick(localx - children[i]->Position.X, localy - children[i]->Position.Y, button);
				break;
			}
		}
	}
	
	//if a child wasn't clicked, send click to ourself
	if(!childunclicked)
	{
		XOnMouseUnclick(localx, localy, button);
	}
}

void Panel::OnMouseUp(int x, int y, unsigned button)
{
	XOnMouseUp(x, y, button);
	for(int i = 0; i < children.size(); ++i)
	{
		if(!children[i]->Locked)
			children[i]->OnMouseUp(x, y, button);
	}
}

void Panel::OnMouseWheel(int localx, int localy, int d)
{
	XOnMouseWheel(localx, localy, d);
	for(int i = 0; i < children.size(); ++i)
	{
		if(!children[i]->Locked)
			children[i]->OnMouseWheel(localx - children[i]->Position.X, localy - children[i]->Position.Y, d);
	}
}

void Panel::OnMouseWheelInside(int localx, int localy, int d)
{
	XOnMouseWheelInside(localx, localy, d);
	//check if clicked a child
	for(int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be unlocked
		if(!children[i]->Locked)
		{
			//is mouse inside?
			if( localx >= children[i]->Position.X &&
				localy >= children[i]->Position.Y &&
				localx < children[i]->Position.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + children[i]->Size.Y )
			{
				children[i]->OnMouseWheelInside(localx - children[i]->Position.X, localy - children[i]->Position.Y, d);
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

void Panel::XOnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
}

void Panel::XOnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
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
