/*
 * Component.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include "interface/Component.h"

namespace ui {

Component::Component(int x, int y, int width, int height):
    X(x),
    Y(y),
    Width(width),
    Height(height),
    Enabled(true),
    Visible(true)
{
}

Component::~Component()
{
}

void Component::Draw(void* userdata)
{
}

void Component::Tick(float dt)
{
}

void Component::OnKeyPress(int key, bool shift, bool ctrl, bool alt)
{
}

void Component::OnKeyRelease(int key, bool shift, bool ctrl, bool alt)
{
}

void Component::OnMouseEnter(int localx, int localy, int dx, int dy)
{
}

void Component::OnMouseLeave(int localx, int localy, int dx, int dy)
{
}

void Component::OnMouseClick(int localx, int localy, unsigned int button)
{
}

void Component::OnMouseUnclick(int localx, int localy, unsigned int button)
{
}

void Component::OnMouseDown(int localx, int localy, unsigned int button)
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

void Component::OnMouseUp(int localx, int localy, unsigned int button)
{
}

void Component::OnMouseWheel(int localx, int localy, int d)
{
}

void Component::OnMouseWheelInside(int localx, int localy, int d)
{
}

void Component::OnMouseWheelFocused(int localx, int localy, int d)
{
}
} /* namespace ui */
