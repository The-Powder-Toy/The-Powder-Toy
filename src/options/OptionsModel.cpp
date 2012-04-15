/*
 * OptionsModel.cpp
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

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
	return false;
	//sim->
}

void OptionsModel::SetNewtonianGravity(bool state)
{
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

