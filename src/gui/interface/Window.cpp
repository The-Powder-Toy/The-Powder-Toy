#include <iostream>
#include "Window.h"
#include "Keys.h"
#include "Component.h"
#include "gui/interface/Point.h"
#include "gui/interface/Button.h"

using namespace ui;

Window::Window(Point _position, Point _size):
	Position(_position),
	Size(_size),
	focusedComponent_(NULL),
	AllowExclusiveDrawing(true),
	halt(false),
	destruct(false),
	stop(false),
	cancelButton(NULL),
	okayButton(NULL)
#ifdef DEBUG
	,debugMode(false)
#endif
{
}

Window::~Window()
{
	for(unsigned i = 0, sz = Components.size(); i < sz; ++i)
		if( Components[i] )
		{
			delete Components[i];
			if(Components[i]==focusedComponent_)
				focusedComponent_ = NULL;
		}
	Components.clear();
}

void Window::AddComponent(Component* c)
{
	// TODO: do a check if component was already added?
	if(c->GetParentWindow()==NULL)
	{
		c->SetParentWindow(this);
		Components.push_back(c);

		if(Engine::Ref().GetMouseX() > Position.X + c->Position.X && Engine::Ref().GetMouseX() < Position.X + c->Position.X + c->Size.X &&
			Engine::Ref().GetMouseY() > Position.Y + c->Position.Y && Engine::Ref().GetMouseY() < Position.Y + c->Position.Y + c->Size.Y)
			c->OnMouseEnter(Engine::Ref().GetMouseX() - (Position.X + c->Position.X), Engine::Ref().GetMouseY() - (Position.Y + c->Position.Y));
	}
	else
	{
		//Component already has a state, don't sad it
	}
}

unsigned Window::GetComponentCount()
{
	return Components.size();
}

Component* Window::GetComponent(unsigned idx)
{
	return Components[idx];
}

void Window::RemoveComponent(Component* c)
{
	// remove component WITHOUT freeing it.
	for(unsigned i = 0; i < Components.size(); ++i)
	{
		// find the appropriate component index
		if(Components[i] == c)
		{
			//Make sure any events don't continue
			halt = true;
			if(Components[i]==focusedComponent_)
				focusedComponent_ = NULL;

			Components.erase(Components.begin() + i);

			// we're done
			return;
		}
	}
}

void Window::OnTryExit(ExitMethod method)
{
	if(cancelButton && method != MouseOutside)
		cancelButton->DoAction();
}

void Window::OnTryOkay(OkayMethod method)
{
	if(okayButton)
		okayButton->DoAction();
}

void Window::RemoveComponent(unsigned idx)
{
	halt = true;
	// free component and remove it.
	if(Components[idx]==focusedComponent_)
		focusedComponent_ = NULL;
	delete Components[idx];
	Components.erase(Components.begin() + idx);
}

bool Window::IsFocused(const Component* c) const
{
	return c == focusedComponent_;
}

void Window::FocusComponent(Component* c)
{
	this->focusedComponent_ = c;
}

void Window::DoExit()
{

	OnExit();
}

void Window::DoInitialized()
{

	OnInitialized();
}

void Window::DoBlur()
{

	OnBlur();
}

void Window::DoFocus()
{

	OnFocus();
}

void Window::DoDraw()
{
	OnDraw();
	//draw
	for(int i = 0, sz = Components.size(); i < sz; ++i)
		if(Components[i]->Visible)
		{
			if(AllowExclusiveDrawing)
			{
				Point scrpos(Components[i]->Position.X + Position.X, Components[i]->Position.Y + Position.Y);
				Components[i]->Draw(scrpos);
			}
			else
			{
				if( Components[i]->Position.X+Position.X + Components[i]->Size.X >= 0 &&
					Components[i]->Position.Y+Position.Y + Components[i]->Size.Y >= 0 &&
					Components[i]->Position.X+Position.X < ui::Engine::Ref().GetWidth() &&
					Components[i]->Position.Y+Position.Y < ui::Engine::Ref().GetHeight() )
				{
					Point scrpos(Components[i]->Position.X + Position.X, Components[i]->Position.Y + Position.Y);
					Components[i]->Draw( Point(scrpos) );
				}
			}
#ifdef DEBUG
			if(debugMode)
			{
				if(focusedComponent_==Components[i])
				{
					ui::Engine::Ref().g->fillrect(Components[i]->Position.X+Position.X, Components[i]->Position.Y+Position.Y, Components[i]->Size.X, Components[i]->Size.Y, 0, 255, 0, 90);
				}
				else
				{
					ui::Engine::Ref().g->fillrect(Components[i]->Position.X+Position.X, Components[i]->Position.Y+Position.Y, Components[i]->Size.X, Components[i]->Size.Y, 255, 0, 0, 90);
				}
			}
#endif
		}
#ifdef DEBUG
	if(debugMode)
	{
		if(focusedComponent_)
		{
			int xPos = focusedComponent_->Position.X+focusedComponent_->Size.X+5+Position.X;
			Graphics * g = ui::Engine::Ref().g;
			char tempString[512];
			char tempString2[512];
			
			sprintf(tempString, "Position: L %d, R %d, T: %d, B: %d", focusedComponent_->Position.X, Size.X-(focusedComponent_->Position.X+focusedComponent_->Size.X), focusedComponent_->Position.Y, Size.Y-(focusedComponent_->Position.Y+focusedComponent_->Size.Y));
			sprintf(tempString2, "Size: %d, %d", focusedComponent_->Size.X, focusedComponent_->Size.Y);
			
			if(Graphics::textwidth(tempString)+xPos > WINDOWW)
				xPos = WINDOWW-(Graphics::textwidth(tempString)+5);
			if(Graphics::textwidth(tempString2)+xPos > WINDOWW)
				xPos = WINDOWW-(Graphics::textwidth(tempString2)+5);
			
			g->drawtext(xPos, focusedComponent_->Position.Y+Position.Y+1, tempString, 0, 0, 0, 200);
			g->drawtext(xPos, focusedComponent_->Position.Y+Position.Y, tempString, 255, 255, 255, 255);
			g->drawtext(xPos, focusedComponent_->Position.Y+Position.Y+13, tempString2, 0, 0, 0, 200);
			g->drawtext(xPos, focusedComponent_->Position.Y+Position.Y+12, tempString2, 255, 255, 255, 255);
		}
		return;
	}
#endif

}

void Window::DoTick(float dt)
{
#ifdef DEBUG
	if(debugMode)
		return;
#endif
	//on mouse hover
	for(int i = Components.size() - 1; i >= 0 && !halt; --i)
	{
		if(!Components[i]->Locked &&
			ui::Engine::Ref().GetMouseX() >= Components[i]->Position.X+Position.X &&
			ui::Engine::Ref().GetMouseY() >= Components[i]->Position.Y+Position.Y &&
			ui::Engine::Ref().GetMouseX() < Components[i]->Position.X+Position.X + Components[i]->Size.X &&
			ui::Engine::Ref().GetMouseY() < Components[i]->Position.Y+Position.Y + Components[i]->Size.Y )
		{
			Components[i]->OnMouseHover(ui::Engine::Ref().GetMouseX() - (Components[i]->Position.X + Position.X), ui::Engine::Ref().GetMouseY() - (Components[i]->Position.Y + Position.Y));
			break;
		}
	}

	//tick
	for(int i = 0, sz = Components.size(); i < sz && !halt; ++i)
	{
		Components[i]->Tick(dt);
	}

	halt = false;
	stop = false;

	OnTick(dt);

	if(destruct)
		finalise();
}

void Window::DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
#ifdef DEBUG
	if(key == KEY_TAB && ctrl)
		debugMode = !debugMode;
	if(debugMode)
	{
		if(focusedComponent_!=NULL)
		{
			if(shift)
			{
				if(key == KEY_UP)
					focusedComponent_->Size.Y--;
				if(key == KEY_DOWN)
					focusedComponent_->Size.Y++;
				if(key == KEY_LEFT)
					focusedComponent_->Size.X--;
				if(key == KEY_RIGHT)
					focusedComponent_->Size.X++;
			}
			if(ctrl)
			{
				if(key == KEY_UP)
					focusedComponent_->Size.Y++;
				if(key == KEY_DOWN)
					focusedComponent_->Size.Y--;
				if(key == KEY_LEFT)
					focusedComponent_->Size.X++;
				if(key == KEY_RIGHT)
					focusedComponent_->Size.X--;
			}
			if(!shift)
			{
				if(key == KEY_UP)
					focusedComponent_->Position.Y--;
				if(key == KEY_DOWN)
					focusedComponent_->Position.Y++;
				if(key == KEY_LEFT)
					focusedComponent_->Position.X--;
				if(key == KEY_RIGHT)
					focusedComponent_->Position.X++;
			}
			if (key == KEY_DELETE)
			{
				RemoveComponent(focusedComponent_);
				halt = false;
			}
		}
		else
		{
			if(shift)
			{
				if(key == KEY_UP)
					Size.Y--;
				if(key == KEY_DOWN)
					Size.Y++;
				if(key == KEY_LEFT)
					Size.X--;
				if(key == KEY_RIGHT)
					Size.X++;
			}
			if(ctrl)
			{
				if(key == KEY_UP)
					Size.Y++;
				if(key == KEY_DOWN)
					Size.Y--;
				if(key == KEY_LEFT)
					Size.X++;
				if(key == KEY_RIGHT)
					Size.X--;
			}
			if(!shift)
			{
				if(key == KEY_UP)
					Position.Y--;
				if(key == KEY_DOWN)
					Position.Y++;
				if(key == KEY_LEFT)
					Position.X--;
				if(key == KEY_RIGHT)
					Position.X++;
			}
		}
		return;
	}
#endif
	//on key press
	if(focusedComponent_ != NULL)
	{
		if(!focusedComponent_->Locked && focusedComponent_->Visible)
			focusedComponent_->OnKeyPress(key, character, shift, ctrl, alt);
	}

	if(!stop)
		OnKeyPress(key, character, shift, ctrl, alt);
	
	if(key == KEY_ESCAPE)
		OnTryExit(Escape);

	if(key == KEY_ENTER || key == KEY_RETURN)
		OnTryOkay(Enter);

	if(destruct)
		finalise();
}

void Window::DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
#ifdef DEBUG
	if(debugMode)
		return;
#endif
	//on key unpress
	if(focusedComponent_ != NULL)
	{
		if(!focusedComponent_->Locked && focusedComponent_->Visible)
			focusedComponent_->OnKeyRelease(key, character, shift, ctrl, alt);
	}

	if(!stop)
		OnKeyRelease(key, character, shift, ctrl, alt);
	if(destruct)
		finalise();
}

void Window::DoMouseDown(int x_, int y_, unsigned button)
{
	//on mouse click
	int x = x_ - Position.X;
	int y = y_ - Position.Y;
	bool clickState = false;
	for(int i = Components.size() - 1; i > -1 && !halt; --i)
	{
		if(!Components[i]->Locked && Components[i]->Visible)
		{
			if(x >= Components[i]->Position.X && y >= Components[i]->Position.Y && x < Components[i]->Position.X + Components[i]->Size.X && y < Components[i]->Position.Y + Components[i]->Size.Y)
			{
				FocusComponent(Components[i]);
#ifdef DEBUG
				if(!debugMode)
#endif
				Components[i]->OnMouseClick(x - Components[i]->Position.X, y - Components[i]->Position.Y, button);
				clickState = true;
				break;
			}
		}
	}

	if(!clickState)
		FocusComponent(NULL);
	
#ifdef DEBUG
	if(debugMode)
		return;
#endif

	//on mouse down
	for(int i = Components.size() - 1; i > -1 && !halt; --i)
	{
		if(!Components[i]->Locked && Components[i]->Visible)
			Components[i]->OnMouseDown(x, y, button);
	}

	if(!stop)
		OnMouseDown(x_, y_, button);

	if(!clickState && (x_ < Position.X || y_ < Position.Y || x_ > Position.X+Size.X || y_ > Position.Y+Size.Y))
		OnTryExit(MouseOutside);

	if(destruct)
		finalise();
}

void Window::DoMouseMove(int x_, int y_, int dx, int dy)
{
	//on mouse move (if true, and inside)
	int x = x_ - Position.X;
	int y = y_ - Position.Y;
#ifdef DEBUG
	if(debugMode)
		return;
#endif
	for(int i = Components.size() - 1; i > -1  && !halt; --i)
	{
		if(!Components[i]->Locked && Components[i]->Visible)
		{
			Point local	(x - Components[i]->Position.X, y - Components[i]->Position.Y)
			, a (local.X - dx, local.Y - dy);

			Components[i]->OnMouseMoved(local.X, local.Y, dx, dy);

			if(local.X >= 0 &&
			   local.Y >= 0 &&
			   local.X < Components[i]->Size.X &&
			   local.Y < Components[i]->Size.Y && !halt)
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
			else if(!halt)
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

	if(!stop)
		OnMouseMove(x_, y_, dx, dy);
	if(destruct)
		finalise();
}

void Window::DoMouseUp(int x_, int y_, unsigned button)
{
	int x = x_ - Position.X;
	int y = y_ - Position.Y;
#ifdef DEBUG
	if(debugMode)
		return;
#endif
	//on mouse unclick
	for(int i = Components.size() - 1; i >= 0  && !halt; --i)
	{
		if(!Components[i]->Locked && Components[i]->Visible)
		{
			if(x >= Components[i]->Position.X && y >= Components[i]->Position.Y && x < Components[i]->Position.X + Components[i]->Size.X && y < Components[i]->Position.Y + Components[i]->Size.Y)
			{
				Components[i]->OnMouseUnclick(x - Components[i]->Position.X, y - Components[i]->Position.Y, button);
				break;
			}
		}
	}

	//on mouse up
	for(int i = Components.size() - 1; i >= 0 && !halt; --i)
	{
		if(!Components[i]->Locked && Components[i]->Visible)
			Components[i]->OnMouseUp(x, y, button);
	}

	if(!stop)
		OnMouseUp(x_, y_, button);
	if(destruct)
		finalise();
}

void Window::DoMouseWheel(int x_, int y_, int d)
{
	int x = x_ - Position.X;
	int y = y_ - Position.Y;
#ifdef DEBUG
	if(debugMode)
		return;
#endif
	//on mouse wheel focused
	for(int i = Components.size() - 1; i >= 0  && !halt; --i)
	{
		if(x >= Components[i]->Position.X && y >= Components[i]->Position.Y && x < Components[i]->Position.X + Components[i]->Size.X && y < Components[i]->Position.Y + Components[i]->Size.Y)
		{
			if(!Components[i]->Locked && Components[i]->Visible)
				Components[i]->OnMouseWheelInside(x - Components[i]->Position.X, y - Components[i]->Position.Y, d);
			break;
		}
	}

	//on mouse wheel
	for(int i = Components.size() - 1; i >= 0  && !halt; --i)
	{
		if(!Components[i]->Locked && Components[i]->Visible)
			Components[i]->OnMouseWheel(x - Components[i]->Position.X, y - Components[i]->Position.Y, d);
	}

	if(!stop)
		OnMouseWheel(x_, y_, d);

	if(destruct)
		finalise();
}

void Window::finalise()
{
	delete this;
}

void Window::SelfDestruct()
{
	destruct = true;
	halt = true;
	stop = true;
}

void Window::Halt()
{
	stop = true;
	halt = true;
}

