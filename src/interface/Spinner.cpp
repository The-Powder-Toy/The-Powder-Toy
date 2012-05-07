/*
 * Spinner.cpp
 *
 *  Created on: Feb 11, 2012
 *      Author: Simon
 */


#include <cmath>
#include <iostream>
#include "Spinner.h"

using namespace ui;

Spinner::Spinner(Point position, Point size):
	Component(position, size), cValue(0)
{
}
void Spinner::Tick(float dt)
{
	cValue += 0.05f;
}
void Spinner::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	int baseX = screenPos.X+(Size.X/2);
	int baseY = screenPos.Y+(Size.Y/2);
	for(float t = 0.0f; t < 1.0f; t+=0.05f)
	{
		//g->drawblob(baseX+(sin(cValue+t)*(Size.X/2)), baseY+(cos(cValue+t)*(Size.X/2)), t*255, t*255, t*255);
	}
}
Spinner::~Spinner()
{

}
