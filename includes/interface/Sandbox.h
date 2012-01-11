/*
 * Sandbox.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#ifndef SANDBOX_H_
#define SANDBOX_H_

#include "Component.h"
#include "Simulation.h"
#include "Renderer.h"

namespace ui {

class Sandbox: public ui::Component {
private:
	int lastCoordX, lastCoordY;
	int activeElement;
	bool isMouseDown;
	Renderer * ren;
	Simulation * sim;
public:
	Sandbox();
	virtual Simulation * GetSimulation();
	virtual void OnMouseMovedInside(int localx, int localy, int dx, int dy);
	virtual void OnMouseDown(int localx, int localy, unsigned int button);
	virtual void OnMouseUp(int localx, int localy, unsigned int button);
	virtual void Draw(void* userdata);
	virtual void Tick(float delta);
	virtual ~Sandbox();
};

} /* namespace ui */
#endif /* SANDBOX_H_ */
