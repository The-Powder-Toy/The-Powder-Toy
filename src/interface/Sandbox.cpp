/*
 * Sandbox.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include "Config.h"

#include "interface/Sandbox.h"
#include "interface/Component.h"
#include "Renderer.h"

namespace ui {

Sandbox::Sandbox():
		Component(0, 0, XRES, YRES)
{
	sim = new Simulation();
}

void Sandbox::OnMouseMovedInside(int localx, int localy, int dx, int dy)
{
	if(isMouseDown)
	{
		sim->create_parts(localx, localy, 20, 20, 1, 0);
	}
}

void Sandbox::OnMouseDown(int localx, int localy, unsigned int button)
{
	isMouseDown = true;
}

void Sandbox::OnMouseUp(int localx, int localy, unsigned int button)
{
	isMouseDown = false;
}

void Sandbox::Draw(void* userdata)
{
	Graphics * g = reinterpret_cast<Graphics*>(userdata);
	if(!ren)
		ren = new Renderer(g, sim);
	ren->render_parts();
}

void Sandbox::Tick(float delta)
{
	sim->update_particles();
}

Sandbox::~Sandbox() {
	// TODO Auto-generated destructor stub
}

} /* namespace ui */
