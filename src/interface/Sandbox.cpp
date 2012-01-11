/*
 * Sandbox.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <iostream>

#include "Config.h"

#include "interface/Sandbox.h"
#include "interface/Component.h"
#include "Renderer.h"
#include "Simulation.h"

namespace ui {

Sandbox::Sandbox():
	Component(0, 0, XRES, YRES),
	ren(NULL),
	isMouseDown(false),
	activeElement(1)
{
	sim = new Simulation();
}

Simulation * Sandbox::GetSimulation()
{
	return sim;
}

void Sandbox::OnMouseMovedInside(int localx, int localy, int dx, int dy)
{
	if(isMouseDown)
	{
		sim->create_line(lastCoordX, lastCoordY, localx, localy, 2, 2, activeElement, 0);
		lastCoordX = localx;
		lastCoordY = localy;
	}
}

void Sandbox::OnMouseDown(int localx, int localy, unsigned int button)
{
	sim->create_line(localx, localy, localx, localy, 2, 2, activeElement, 0);
	lastCoordX = localx;
	lastCoordY = localy;
	isMouseDown = true;
}

void Sandbox::OnMouseUp(int localx, int localy, unsigned int button)
{
	sim->create_line(lastCoordX, lastCoordY, localx, localy, 2, 2, activeElement, 0);
	lastCoordX = localx;
	lastCoordY = localy;
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
