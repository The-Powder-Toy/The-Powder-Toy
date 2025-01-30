#include "gui/interface/Component.h"

#include "graphics/Graphics.h"
#include "gui/interface/Point.h"
#include "gui/interface/Window.h"
#include "gui/interface/Panel.h"
#include "gui/interface/ContextMenu.h"

using namespace ui;

Component::Component(Point position, Point size):
	parentstate_(nullptr),
	_parent(nullptr),
	drawn(false),
	textPosition(0, 0),
	textSize(0, 0),
	iconPosition(0, 0),
	menu(nullptr),
	Position(position),
	Size(size),
	Enabled(true),
	Visible(true),
	DoesTextInput(false)
{

}

void Component::Refresh()
{
	drawn = false;
}

void Component::TextPosition(String displayText)
{

	textPosition = ui::Point(0, 0);

	textSize = Graphics::TextSize(displayText);
	int textWidth = textSize.X, textHeight = textSize.Y;
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
	if(new_parent == nullptr)
	{
		if(_parent != nullptr)
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

Point Component::GetContainerPos()
{
	Point newPos(0,0);
	if(GetParentWindow())
		newPos += GetParentWindow()->Position;
	if(GetParent())
		newPos += GetParent()->Position + GetParent()->ViewportPosition;
	return newPos;
}

Point Component::GetScreenPos()
{
	return GetContainerPos() + Position;
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

void Component::Tick()
{
}

void Component::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
}

void Component::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
}

void Component::OnTextInput(String text)
{
}

void Component::OnTextEditing(String text)
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

void Component::OnMouseMoved(int localx, int localy)
{
}

void Component::OnMouseEnter(int localx, int localy)
{
}

void Component::OnMouseLeave(int localx, int localy)
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

void Component::OnFocus()
{

}

void Component::OnDefocus()
{

}

Component::~Component()
{
	delete menu;
}
