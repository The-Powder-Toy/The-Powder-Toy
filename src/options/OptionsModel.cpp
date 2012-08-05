/*
 * OptionsModel.cpp
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

#include "simulation/Air.h"
#include "OptionsModel.h"

OptionsModel::OptionsModel(Simulation * sim_) {
	sim = sim_;
}

void OptionsModel::AddObserver(OptionsView* view)
{
	observers.push_back(view);
	view->NotifySettingsChanged(this);
}

bool OptionsModel::GetHeatSimulation()
{
	return sim->legacy_enable?false:true;
}

void OptionsModel::SetHeatSimulation(bool state)
{
	sim->legacy_enable = state?0:1;
	notifySettingsChanged();
}

bool OptionsModel::GetAmbientHeatSimulation()
{
	return sim->aheat_enable?true:false;
}

void OptionsModel::SetAmbientHeatSimulation(bool state)
{
	sim->aheat_enable = state?1:0;
	notifySettingsChanged();
}

bool OptionsModel::GetNewtonianGravity()
{
	return sim->grav->ngrav_enable?true:false;
}

void OptionsModel::SetNewtonianGravity(bool state)
{
	if(state)
		sim->grav->start_grav_async();
	else
		sim->grav->stop_grav_async();
	notifySettingsChanged();
}

bool OptionsModel::GetWaterEqualisation()
{
	return sim->water_equal_test?true:false;
}

void OptionsModel::SetWaterEqualisation(bool state)
{
	sim->water_equal_test = state?1:0;
	notifySettingsChanged();
}

int OptionsModel::GetAirMode()
{
	return sim->air->airMode;
}
void OptionsModel::SetAirMode(int airMode)
{
	sim->air->airMode = airMode;
	notifySettingsChanged();
}

int OptionsModel::GetEdgeMode()
{
	return sim->edgeMode;
}
void OptionsModel::SetEdgeMode(int edgeMode)
{
	sim->SetEdgeMode(edgeMode);
	notifySettingsChanged();
}

int OptionsModel::GetGravityMode()
{
	return sim->gravityMode;
}
void OptionsModel::SetGravityMode(int gravityMode)
{
	sim->gravityMode = gravityMode;
	notifySettingsChanged();
}

void OptionsModel::notifySettingsChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySettingsChanged(this);
	}
}

OptionsModel::~OptionsModel() {
	// TODO Auto-generated destructor stub
}

