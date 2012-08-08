/*
 * OptionsController.h
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

#ifndef OPTIONSCONTROLLER_H_
#define OPTIONSCONTROLLER_H_

#include "Controller.h"
#include "simulation/Simulation.h"
#include "OptionsView.h"
#include "OptionsModel.h"

class OptionsModel;
class OptionsView;
class OptionsController {
	OptionsView * view;
	OptionsModel * model;
	ControllerCallback * callback;
public:
	bool HasExited;
	OptionsController(Simulation * sim, ControllerCallback * callback_);
	void SetHeatSimulation(bool state);
	void SetAmbientHeatSimulation(bool state);
	void SetNewtonianGravity(bool state);
	void SetWaterEqualisation(bool state);
	void SetGravityMode(int gravityMode);
	void SetAirMode(int airMode);
	void SetEdgeMode(int airMode);
	void SetFullscreen(bool fullscreen);
	void SetScale(bool scale);
	void Exit();
	OptionsView * GetView();
	virtual ~OptionsController();
};

#endif /* OPTIONSCONTROLLER_H_ */
