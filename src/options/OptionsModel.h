/*
 * OptionsModel.h
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

#ifndef OPTIONSMODEL_H_
#define OPTIONSMODEL_H_
#include <vector>
#include "OptionsView.h"
#include "simulation/Simulation.h"

class Simulation;
class OptionsView;
class OptionsModel {
	Simulation * sim;
	std::vector<OptionsView*> observers;
	void notifySettingsChanged();
public:
	OptionsModel(Simulation * sim_);
	void AddObserver(OptionsView* view);
	bool GetHeatSimulation();
	void SetHeatSimulation(bool state);
	bool GetAmbientHeatSimulation();
	void SetAmbientHeatSimulation(bool state);
	bool GetNewtonianGravity();
	void SetNewtonianGravity(bool state);
	bool GetWaterEqualisation();
	void SetWaterEqualisation(bool state);
	int GetAirMode();
	void SetAirMode(int airMode);
	int GetEdgeMode();
	void SetEdgeMode(int edgeMode);
	int GetGravityMode();
	void SetGravityMode(int gravityMode);
	bool GetFullscreen();
	void SetFullscreen(bool fullscreen);
	bool GetScale();
	void SetScale(bool scale);
	virtual ~OptionsModel();
};

#endif /* OPTIONSMODEL_H_ */
