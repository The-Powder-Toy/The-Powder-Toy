#include "gui/interface/Panel.h"

#include "gui/interface/Engine.h"
#include "gui/interface/Window.h"

#include "graphics/Graphics.h"

using namespace ui;

Panel::Panel(Point position, Point size):
	Component(position, size),
	InnerSize(size),
	ViewportPosition(0, 0)
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
	c->MouseInside = false;
	c->MouseDownInside = false;
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
				this->GetParentWindow()->FocusComponent(nullptr);
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

	auto clip = RectSized(screenPos, Size);
	GetGraphics()->SwapClipRect(clip);

	// attempt to draw all children
	for (auto const child : children)
		// the component must be visible
		if (child->Visible)
		{
			auto rect = RectSized(child->Position + ViewportPosition, child->Size);
			//check if the component is in the screen, draw if it is
			if (rect & Size.OriginRect())
			{
				child->Draw(screenPos + rect.pos);
			}
		}

	GetGraphics()->SwapClipRect(clip); // apply old cliprect
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
	XOnMouseClick(localx, localy, button);
}

void Panel::OnMouseDown(int x, int y, unsigned button)
{
	if (MouseDownInside)
	{
		auto localx = x - Position.X;
		auto localy = y - Position.Y;
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
					GetParentWindow()->FocusComponent(children[i]);
					children[i]->MouseDownInside = true;
					break;
				}
			}
		}

		XOnMouseDown(x, y, button);
		for (size_t i = 0; i < children.size(); ++i)
		{
			if(children[i]->Enabled)
				children[i]->OnMouseDown(x - Position.X - ViewportPosition.X, y - Position.Y - ViewportPosition.Y, button);
		}
	}
}

void Panel::OnMouseHover(int localx, int localy)
{
	// check if hovering on children
	for (int i = children.size() - 1; i >= 0; --i)
	{
		if (children[i]->Enabled)
		{
			auto px = children[i]->Position.X + ViewportPosition.X;
			auto py = children[i]->Position.Y + ViewportPosition.Y;
			if( localx >= px &&
				localy >= py &&
				localx < px + children[i]->Size.X &&
				localy < py + children[i]->Size.Y )
			{
				children[i]->OnMouseHover(localx - px, localy - py);
				break;
			}
		}
	}

	// always allow hover on parent (?)
	XOnMouseHover(localx, localy);
}

void Panel::OnMouseMoved(int localx, int localy)
{
	PropagateMouseMove();
	XOnMouseMoved(localx, localy);
	for (size_t i = 0; i < children.size(); ++i)
	{
		if(children[i]->Enabled)
			children[i]->OnMouseMoved(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y);
	}
}

void Panel::PropagateMouseMove()
{
	auto localx = ui::Engine::Ref().GetMouseX() - GetScreenPos().X;
	auto localy = ui::Engine::Ref().GetMouseY() - GetScreenPos().Y;
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i]->Enabled)
		{
			Point local	(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y);

			// mouse currently inside?
			if( local.X >= 0 &&
				local.Y >= 0 &&
				local.X < children[i]->Size.X &&
				local.Y < children[i]->Size.Y )
			{
				children[i]->OnMouseMoved(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y);

				// was the mouse outside?
				if (!children[i]->MouseInside)
				{
					children[i]->MouseInside = true;
					children[i]->OnMouseEnter(local.X, local.Y);
				}
			}
			// if not currently inside
			else
			{
				// was the mouse inside?
				if (children[i]->MouseInside)
				{
					children[i]->MouseInside = false;
					children[i]->OnMouseLeave(local.X, local.Y);
				}

			}
		}
	}
}

void Panel::OnMouseEnter(int localx, int localy)
{
	XOnMouseEnter(localx, localy);
}

void Panel::OnMouseLeave(int localx, int localy)
{
	XOnMouseLeave(localx, localy);
}

void Panel::OnMouseUp(int x, int y, unsigned button)
{
	auto localx = x - Position.X;
	auto localy = y - Position.Y;
	//check if clicked a child
	for(int i = children.size()-1; i >= 0 ; --i)
	{
		//child must be enabled
		if(children[i]->Enabled)
		{
			//is mouse inside?
			if( children[i]->MouseDownInside &&
				localx >= children[i]->Position.X + ViewportPosition.X &&
				localy >= children[i]->Position.Y + ViewportPosition.Y &&
				localx < children[i]->Position.X + ViewportPosition.X + children[i]->Size.X &&
				localy < children[i]->Position.Y + ViewportPosition.Y + children[i]->Size.Y )
			{
				children[i]->OnMouseClick(localx - children[i]->Position.X - ViewportPosition.X, localy - children[i]->Position.Y - ViewportPosition.Y, button);
				break;
			}
		}
	}
	for (auto *child : children)
	{
		child->MouseDownInside = false;
	}

	XOnMouseUp(x, y, button);
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i]->Enabled)
			children[i]->OnMouseUp(x - Position.X - ViewportPosition.X, y - Position.Y - ViewportPosition.Y, button);
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

void Panel::XOnMouseMoved(int localx, int localy)
{
}

void Panel::XOnMouseEnter(int localx, int localy)
{
}

void Panel::XOnMouseLeave(int localx, int localy)
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
