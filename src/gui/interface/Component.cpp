#include <iostream>
#include "graphics/Graphics.h"
#include "gui/interface/Component.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Point.h"
#include "gui/interface/Window.h"
#include "gui/interface/Panel.h"
#include "gui/interface/ContextMenu.h"

using namespace ui;

Component::Component(Window* parent_state):
	parentstate_(parent_state),
	_parent(NULL),
	drawn(false),
	textPosition(0, 0),
	textSize(0, 0),
	iconPosition(0, 0),
	menu(NULL),
	Position(Point(0,0)),
	Size(Point(0,0)),
	Enabled(true),
	Visible(true)
{

}

Component::Component(Point position, Point size):
	parentstate_(0),
	_parent(NULL),
	drawn(false),
	textPosition(0, 0),
	textSize(0, 0),
	iconPosition(0, 0),
	menu(NULL),
	Position(position),
	Size(size),
	Enabled(true),
	Visible(true)
{

}

Component::Component():
	parentstate_(NULL),
	_parent(NULL),
	drawn(false),
	textPosition(0, 0),
	textSize(0, 0),
	iconPosition(0, 0),
	menu(NULL),
	Position(Point(0,0)),
	Size(Point(0,0)),
	Enabled(true),
	Visible(true)
{

}

void Component::Refresh()
{
	drawn = false;
}

void Component::TextPosition(std::string displayText)
{

	textPosition = ui::Point(0, 0);
	
	int textWidth, textHeight = 10;
	Graphics::textsize((char*)displayText.c_str(), textWidth, textHeight);
	textSize.X = textWidth; textSize.Y = textHeight;
	textHeight-=3;
	textWidth-=1;
	if(Appearance.icon)
	{
		textWidth += 13;
	}
	
	int textAreaWidth = Size.X-(Appearance.Margin.Right+Appearance.Margin.Left);
	int textAreaHeight = Size.Y-(Appearance.Margin.Top+Appearance.Margin.Bottom);
	
	switch(Appearance.VerticalAlign)
	{
		case ui::Appearance::AlignTop:
			textPosition.Y = Appearance.Margin.Top+2;
			break;
		case ui::Appearance::AlignMiddle:
			textPosition.Y = Appearance.Margin.Top+((textAreaHeight-textHeight)/2);
			break;
		case ui::Appearance::AlignBottom:
			textPosition.Y = Size.Y-(textHeight+Appearance.Margin.Bottom);
			break;
	}
	
	switch(Appearance.HorizontalAlign)
	{
		case ui::Appearance::AlignLeft:
			textPosition.X = Appearance.Margin.Left;
			break;
		case ui::Appearance::AlignCentre:
			textPosition.X = Appearance.Margin.Left+((textAreaWidth-textWidth)/2);
			break;
		case ui::Appearance::AlignRight:
			textPosition.X = Size.X-(textWidth+Appearance.Margin.Right);
			break;
	}
	if(Appearance.icon)
	{
		iconPosition = textPosition-ui::Point(0, 1);
		textPosition.X += 15;
	}
}

bool Component::IsFocused() const
{
	if(parentstate_)
		return parentstate_->IsFocused(this);
	return false;
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

Point Component::GetScreenPos()
{
	Point newPos(0,0);
	if(GetParentWindow())
		newPos += GetParentWindow()->Position;
	if(GetParent())
		newPos += GetParent()->Position + GetParent()->ViewportPosition;
	newPos += Position;
	return newPos;
}

Graphics * Component::GetGraphics()
{
	return parentstate_->GetGraphics();
}

// ***** OVERRIDEABLES *****
// Kept empty.

void Component::OnContextMenuAction(int item)
{

}

void Component::Draw(const Point& screenPos)
{
	drawn = true;
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
	delete menu;
}
