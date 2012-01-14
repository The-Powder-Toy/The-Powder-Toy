#include <vector>
#include "interface/Component.h"
#include "interface/Engine.h"
#include "interface/State.h"
//#include "Platform.h"

using namespace ui;

State::State()
: UserData(NULL)
, focusedComponent_(NULL)
{
}

State::~State()
{
	for(unsigned i = 0, sz = Components.size(); i < sz; ++i)
		if( Components[i] )
			delete Components[i];
}

void State::AddComponent(Component* c)
{
	// TODO: do a check if component was already added?
	if(c->GetParentState()==NULL)
	{
		c->SetParentState(this);
		Components.push_back(c);
	}
	else
	{
		//Component already has a state, don't sad it
	}
}

unsigned State::GetComponentCount()
{
	return Components.size();
}

Component* State::GetComponent(unsigned idx)
{
	return Components[idx];
}

void State::RemoveComponent(Component* c)
{
	// remove component WITHOUT freeing it.
	for(unsigned i = 0; i < Components.size(); ++i)
	{
		// find the appropriate component index
		if(Components[i] == c)
		{
			Components.erase(Components.begin() + i);
			
			// we're done
			return;
		}
	}
}

void State::RemoveComponent(unsigned idx)
{
	// free component and remove it.
	delete Components[idx];
	Components.erase(Components.begin() + idx);
}

bool State::IsFocused(const Component* c) const
{
	return c == focusedComponent_;
}

void State::FocusComponent(Component* c)
{
	this->focusedComponent_ = c;
}

void State::DoExit()
{

	OnExit();
}

void State::DoInitialized()
{

	OnInitialized();
}

void State::DoDraw()
{
	//draw
	for(int i = 0, sz = Components.size(); i < sz; ++i)
		if(Components[i]->Visible)
		{
			if(AllowExclusiveDrawing)
			{
				Point scrpos(Components[i]->Position.X, Components[i]->Position.Y);
				Components[i]->Draw(scrpos);
			}
			else
			{
				if( Components[i]->Position.X + Components[i]->Size.X >= 0 &&
					Components[i]->Position.Y + Components[i]->Size.Y >= 0 &&
					Components[i]->Position.X < ui::Engine::Ref().GetWidth() &&
					Components[i]->Position.Y < ui::Engine::Ref().GetHeight() )
				{
					Point scrpos(Components[i]->Position.X, Components[i]->Position.Y);
					Components[i]->Draw( Point(scrpos) );
				}
			}
		}

	OnDraw();
}

void State::DoTick(float dt)
{
	//on mouse hover
	for(int i = Components.size() - 1; i >= 0; --i)
	{
		if(!Components[i]->Locked &&
			ui::Engine::Ref().GetMouseX() >= Components[i]->Position.X &&
			ui::Engine::Ref().GetMouseY() >= Components[i]->Position.Y &&
			ui::Engine::Ref().GetMouseX() < Components[i]->Position.X + Components[i]->Size.X &&
			ui::Engine::Ref().GetMouseY() < Components[i]->Position.Y + Components[i]->Size.Y )
		{
			Components[i]->OnMouseHover(ui::Engine::Ref().GetMouseX() - Components[i]->Position.X, ui::Engine::Ref().GetMouseY() - Components[i]->Position.Y);
			break;
		}
	}
			
	//tick
	for(int i = 0, sz = Components.size(); i < sz; ++i)
	{
		Components[i]->Tick(dt);
	}

	OnTick(dt);
}

void State::DoKeyPress(int key, bool shift, bool ctrl, bool alt)
{
	//on key press
	if(focusedComponent_ != NULL)
	{
		if(!focusedComponent_->Locked)
			focusedComponent_->OnKeyPress(key, shift, ctrl, alt);
	}

	OnKeyPress(key, shift, ctrl, alt);
}

void State::DoKeyRelease(int key, bool shift, bool ctrl, bool alt)
{
	//on key unpress
	if(focusedComponent_ != NULL)
	{
		if(!focusedComponent_->Locked)
			focusedComponent_->OnKeyRelease(key, shift, ctrl, alt);
	}

	OnKeyRelease(key, shift, ctrl, alt);
}

void State::DoMouseDown(int x, int y, unsigned button)
{
	//on mouse click
	bool clickState = false;
	for(int i = Components.size() - 1; i > -1 ; --i)
	{
		if(!Components[i]->Locked)
		{
			if(x >= Components[i]->Position.X && y >= Components[i]->Position.Y && x < Components[i]->Position.X + Components[i]->Size.X && y < Components[i]->Position.Y + Components[i]->Size.Y)
			{
				FocusComponent(Components[i]);
				Components[i]->OnMouseClick(x - Components[i]->Position.X, y - Components[i]->Position.Y, button);
				clickState = true;
				break;
			}
		}
	}

	if(!clickState)
		FocusComponent(NULL);

	//on mouse down
	for(int i = Components.size() - 1; i > -1 ; --i)
	{
		if(!Components[i]->Locked)
			Components[i]->OnMouseDown(x, y, button);
	}

	OnMouseDown(x, y, button);
}

void State::DoMouseMove(int x, int y, int dx, int dy)
{
	//on mouse move (if true, and inside)
	for(int i = Components.size() - 1; i > -1 ; --i)
	{
		if(!Components[i]->Locked)
		{
			Point local	(x - Components[i]->Position.X, y - Components[i]->Position.Y)
			, a (local.X - dx, local.Y - dy);
			
			Components[i]->OnMouseMoved(local.X, local.Y, dx, dy);
			
			if(local.X >= 0 &&
			   local.Y >= 0 &&
			   local.X < Components[i]->Size.X &&
			   local.Y < Components[i]->Size.Y )
			{
				Components[i]->OnMouseMovedInside(local.X, local.Y, dx, dy);
				
				// entering?
				if(!(
					a.X >= 0 &&
					a.Y >= 0 &&
					a.X < Components[i]->Size.X &&
					a.Y < Components[i]->Size.Y ))
				{
					Components[i]->OnMouseEnter(local.X, local.Y);
				}
			}
			else
			{
				// leaving?
				if(	a.X >= 0 &&
					a.Y >= 0 &&
					a.X < Components[i]->Size.X &&
					a.Y < Components[i]->Size.Y )
				{
					Components[i]->OnMouseLeave(local.X, local.Y);
				}
				
			}
		}
	}

	OnMouseMove(x, y, dx, dy);
}

void State::DoMouseUp(int x, int y, unsigned button)
{
	//on mouse unclick
	for(int i = Components.size() - 1; i >= 0 ; --i)
	{
		if(!Components[i]->Locked)
		{
			if(x >= Components[i]->Position.X && y >= Components[i]->Position.Y && x < Components[i]->Position.X + Components[i]->Size.X && y < Components[i]->Position.Y + Components[i]->Size.Y)
			{
				Components[i]->OnMouseUnclick(x - Components[i]->Position.X, y - Components[i]->Position.Y, button);
				break;
			}
		}
	}

	//on mouse up
	for(int i = Components.size() - 1; i >= 0 ; --i)
	{
		if(!Components[i]->Locked)
			Components[i]->OnMouseUp(x, y, button);
	}

	OnMouseUp(x, y, button);
}

void State::DoMouseWheel(int x, int y, int d)
{
	//on mouse wheel focused
	for(int i = Components.size() - 1; i >= 0 ; --i)
	{
		if(x >= Components[i]->Position.X && y >= Components[i]->Position.Y && x < Components[i]->Position.X + Components[i]->Size.X && y < Components[i]->Position.Y + Components[i]->Size.Y)
		{
			if(!Components[i]->Locked)
				Components[i]->OnMouseWheelInside(x - Components[i]->Position.X, y - Components[i]->Position.Y, d);
			break;
		}
	}
	
	//on mouse wheel
	for(int i = Components.size() - 1; i >= 0 ; --i)
	{
		if(!Components[i]->Locked)
			Components[i]->OnMouseWheel(x - Components[i]->Position.X, y - Components[i]->Position.Y, d);
	}

	OnMouseWheel(x, y, d);
}
