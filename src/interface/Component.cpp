//#include "Platform.h"
#include "interface/Component.h"
#include "interface/Engine.h"
#include "interface/Point.h"
#include "interface/Window.h"
#include "interface/Panel.h"

using namespace ui;

Component::Component(Window* parent_state):
	parentstate_(parent_state),
	_parent(NULL),
	Position(Point(0,0)),
	Size(Point(0,0)),
	Locked(false),
	Visible(true)
{

}

Component::Component(Point position, Point size):
	parentstate_(NULL),
	_parent(NULL),
	Position(position),
	Size(size),
	Locked(false),
	Visible(true)
{

}

Component::Component():
	parentstate_(NULL),
	_parent(NULL),
	Position(Point(0,0)),
	Size(Point(0,0)),
	Locked(false),
	Visible(true)
{

}

bool Component::IsFocused() const
{
	return parentstate_->IsFocused(this);
}

void Component::SetParentWindow(Window* window)
{
	parentstate_ = window;
}

void Component::SetParent(Panel* new_parent)
{
	if(new_parent == NULL)
	{
		if(_parent != NULL)
		{
			// remove from current parent and send component to parent state
			for(int i = 0; i < _parent->GetChildCount(); ++i)
			{
				if(_parent->GetChild(i) == this)
				{
					// remove ourself from parent component
					_parent->RemoveChild(i, false);
					
					// add ourself to the parent state
					GetParentWindow()->AddComponent(this);
					
					//done in this loop.
					break;
				}
			}
		}
	}
	else
	{
		// remove from parent state (if in parent state) and place in new parent
		if(GetParentWindow())
			GetParentWindow()->RemoveComponent(this);
		new_parent->children.push_back(this);
	}
	this->_parent = new_parent;
}

// ***** OVERRIDEABLES *****
// Kept empty.

void Component::Draw(const Point& screenPos)
{
}

void Component::Tick(float dt)
{
}

void Component::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
}

void Component::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
}

void Component::OnMouseClick(int localx, int localy, unsigned button)
{
}

void Component::OnMouseDown(int x, int y, unsigned button)
{
}

void Component::OnMouseHover(int localx, int localy)
{
}

void Component::OnMouseMoved(int localx, int localy, int dx, int dy)
{
}

void Component::OnMouseMovedInside(int localx, int localy, int dx, int dy)
{
}

void Component::OnMouseEnter(int localx, int localy)
{
}

void Component::OnMouseLeave(int localx, int localy)
{
}

void Component::OnMouseUnclick(int localx, int localy, unsigned button)
{
}

void Component::OnMouseUp(int x, int y, unsigned button)
{
}

void Component::OnMouseWheel(int localx, int localy, int d)
{
}

void Component::OnMouseWheelInside(int localx, int localy, int d)
{
}

Component::~Component()
{
	if(GetParentWindow()->IsFocused(this))
		GetParentWindow()->FocusComponent(NULL);
}
