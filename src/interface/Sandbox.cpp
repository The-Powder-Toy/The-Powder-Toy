/*
 * Sandbox.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <iostream>
#include <queue>

#include "Config.h"
#include "Global.h"

#include "interface/Point.h"
#include "interface/Sandbox.h"
#include "interface/Component.h"
#include "Renderer.h"
#include "Simulation.h"

namespace ui {

Sandbox::Sandbox():
	Component(Point(0, 0), Point(XRES, YRES)),
	pointQueue(std::queue<Point*>()),
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

void Sandbox::OnMouseMoved(int localx, int localy, int dx, int dy)
{
	if(isMouseDown)
	{
		pointQueue.push(new Point(localx-dx, localy-dy));
		pointQueue.push(new Point(localx, localy));
	}
}

void Sandbox::OnMouseClick(int localx, int localy, unsigned int button)
{
	isMouseDown = true;
	pointQueue.push(new Point(localx, localy));
}

void Sandbox::OnMouseUp(int localx, int localy, unsigned int button)
{
	if(isMouseDown)
	{
		isMouseDown = false;
		pointQueue.push(new Point(localx, localy));
	}
}

void Sandbox::Draw(const Point& screenPos)
{
	Graphics * g = Global::Ref().g;
	if(!ren)
		ren = new Renderer(g, sim);
	ren->render_parts();
}

void Sandbox::Tick(float delta)
{
	if(!pointQueue.empty())
	{
		Point * sPoint = NULL;
		while(!pointQueue.empty())
		{
			Point * fPoint = pointQueue.front();
			pointQueue.pop();
			if(sPoint)
			{
				sim->create_line(fPoint->X, fPoint->Y, sPoint->X, sPoint->Y, 1, 1, activeElement, 0);
				delete sPoint;
			}
			else
			{
				sim->create_parts(fPoint->X, fPoint->Y, 1, 1, activeElement, 0);
			}
			sPoint = fPoint;
		}
		if(sPoint)
			delete sPoint;
	}
	sim->update_particles();
	sim->sys_pause = 1;
}

Sandbox::~Sandbox() {
	// TODO Auto-generated destructor stub
}

} /* namespace ui */
