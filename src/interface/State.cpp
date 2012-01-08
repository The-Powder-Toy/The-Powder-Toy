/*
 * State.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <vector>
#include <iostream>
#include <cstring>

#include "interface/State.h"

namespace ui {

State::State(int w, int h) :
    mouseX(0),
    mouseY(0),
    mouseXP(0),
    mouseYP(0),
    width(w),
    height(h)
{
}

State::~State()
{
    //Components.~vector(); // just in case // devnote : Nope.jpg Nate :3 -frankbro
}

void State::Add(Component* child)
{
    Components.push_back(child);
    child->Parent = this;
}

void State::Remove(Component* child)
{
    for(int i = 0; i < Components.size(); i++)
        if(Components[i] == child)
        {
            Components.erase(Components.begin() + i);
            break;
        }
}

void State::Draw(void* userdata)
{
    //draw
    for(int i = 0; i < Components.size(); i++)
    {
        if(Components[i]->Visible)
        {
            if(AllowExclusiveDrawing)
            {
                Components[i]->Draw(userdata);
            }
            else if(
                Components[i]->X + Components[i]->Width >= 0 &&
                Components[i]->Y + Components[i]->Height >= 0 &&
                Components[i]->X < width &&
                Components[i]->Y < height )
            {
                Components[i]->Draw(userdata);
            }
        }
    }
}

void State::Tick(float dt)
{
	//on mouse hover
	for(int i = 0; i < Components.size(); i++)
		if( mouseX >= Components[i]->X &&
			mouseY >= Components[i]->Y &&
			mouseX < Components[i]->X + Components[i]->Width &&
			mouseY < Components[i]->Y + Components[i]->Height )
        {
		    if(Components[i]->Enabled)
		    {
                Components[i]->OnMouseHover(mouseX, mouseY);
		    }
			break;
		}

	//tick
	for(int i = 0; i < Components.size(); i++)
		Components[i]->Tick(dt);
}

void State::OnKeyPress(int key, bool shift, bool ctrl, bool alt)
{
	//on key press
	if(focusedComponent_ != NULL)
		if(focusedComponent_->Enabled)
			focusedComponent_->OnKeyPress(key, shift, ctrl, alt);
}

void State::OnKeyRelease(int key, bool shift, bool ctrl, bool alt)
{
	//on key unpress
	if(focusedComponent_ != NULL)
		if(focusedComponent_->Enabled)
			focusedComponent_->OnKeyRelease(key, shift, ctrl, alt);
}

void State::OnMouseDown(int x, int y, unsigned int button)
{
	//on mouse click
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(Components[i]->Enabled)
			if(x >= Components[i]->X && y >= Components[i]->Y && x < Components[i]->X + Components[i]->Width && y < Components[i]->Y + Components[i]->Height)
			{
				Components[i]->OnMouseClick(x - Components[i]->X, y - Components[i]->Y, button);
				this->focusedComponent_ = Components[i]; //set this component as the focused component
				break;
			}

	//on mouse down
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(Components[i]->Enabled)
			Components[i]->OnMouseDown(x - Components[i]->X, y - Components[i]->Y, button);
}

void State::OnMouseMove(int x, int y)
{
    //update mouse coords
    mouseX = x;
    mouseY = y;

	//on mouse move (if true, and inside)
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(Components[i]->Enabled)
		{
			int localX  = x - Components[i]->X;
			int localY  = y - Components[i]->Y;
			int localXP = mouseXP - Components[i]->X;
			int localYP = mouseYP - Components[i]->Y;
			int dx      = x - mouseXP;
			int dy      = x - mouseYP;

			Components[i]->OnMouseMoved(localX, localY, dx, dy);

            //is the mouse inside
			if(localX >= 0 &&
			   localY >= 0 &&
			   localX < Components[i]->Width &&
			   localY < Components[i]->Height )
            {
                //was the mouse outside last tick?
                if(localXP < 0 ||
                   localXP >= Components[i]->Width ||
                   localYP < 0 ||
                   localYP >= Components[i]->Height )
                {
                    Components[i]->OnMouseEnter(localX, localY, dx, dy);
                }

				Components[i]->OnMouseMovedInside(localX, localY, dx, dy);

				break; //found the top-most component under mouse, break that shit
            }
            //not inside, let's see if it used to be inside last tick
            else if (localXP >= 0 &&
			   localYP >= 0 &&
			   localXP < Components[i]->Width &&
			   localYP < Components[i]->Height )
           {
               Components[i]->OnMouseLeave(localX, localY, x - mouseXP, y - mouseYP);
           }
		}
		else //is locked
		{
		    int localX = x - Components[i]->X;
			int localY = y - Components[i]->Y;

		    //is the mouse inside
			if(localX >= 0 &&
			   localY >= 0 &&
			   localX < Components[i]->Width &&
			   localY < Components[i]->Height )
            {
                break; //it's the top-most component under the mouse, we don't want to go under it.
            }
		}
    // end of for loop here

    //set the previous mouse coords
    mouseXP = x;
    mouseYP = y;
}

void State::OnMouseUp(int x, int y, unsigned int button)
{
	//on mouse unclick
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(Components[i]->Enabled)
			if(x >= Components[i]->X && y >= Components[i]->Y && x < Components[i]->X + Components[i]->Width && y < Components[i]->Y + Components[i]->Height)
			{
				Components[i]->OnMouseUnclick(x - Components[i]->X, y - Components[i]->Y, button);
				break;
			}

	//on mouse up
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(Components[i]->Enabled)
			Components[i]->OnMouseUp(x - Components[i]->X, y - Components[i]->Y, button);
}

void State::OnMouseWheel(int x, int y, int d)
{
    //focused mouse wheel
    if(focusedComponent_ != NULL)
        focusedComponent_->OnMouseWheelFocused(x - focusedComponent_->X, y - focusedComponent_->Y, d);

	//mouse wheel inside
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(x >= Components[i]->X && y >= Components[i]->Y && x < Components[i]->X + Components[i]->Width && y < Components[i]->Y + Components[i]->Height)
		{
			if(Components[i]->Enabled)
				Components[i]->OnMouseWheelInside(x - Components[i]->X, y - Components[i]->Y, d);
			break; //found top-most component under mouse
		}

	//on mouse wheel
	for(int i = Components.size() - 1; i > -1 ; i--)
		if(Components[i]->Enabled)
			Components[i]->OnMouseWheel(x - Components[i]->X, y - Components[i]->Y, d);
}


} /* namespace ui */
