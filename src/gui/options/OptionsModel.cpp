#include "OptionsModel.h"

#include "OptionsView.h"

#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "simulation/Gravity.h"

#include "client/Client.h"

#include "gui/interface/Engine.h"
#include "gui/game/GameModel.h"

OptionsModel::OptionsModel(GameModel * gModel_) {
	gModel = gModel_;
	sim = gModel->GetSimulation();
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
	return sim->grav->IsEnabled();
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
	return gModel->GetSimulation()->edgeMode;
}
void OptionsModel::SetEdgeMode(int edgeMode)
{
	gModel->SetEdgeMode(edgeMode);
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

int OptionsModel::GetScale()
{
	return ui::Engine::Ref().GetScale();
}

void OptionsModel::SetScale(int scale)
{
	ui::Engine::Ref().SetScale(scale);
	Client::Ref().SetPref("Scale", int(scale));
	notifySettingsChanged();
}

bool OptionsModel::GetResizable()
{
	return ui::Engine::Ref().GetResizable();
}

void OptionsModel::SetResizable(bool resizable)
{
	ui::Engine::Ref().SetResizable(resizable);
	Client::Ref().SetPref("Resizable", resizable);
	notifySettingsChanged();
}

bool OptionsModel::GetFullscreen()
{
	return ui::Engine::Ref().GetFullscreen();
}
void OptionsModel::SetFullscreen(bool fullscreen)
{
	ui::Engine::Ref().SetFullscreen(fullscreen);
	Client::Ref().SetPref("Fullscreen", fullscreen);
	notifySettingsChanged();
}

bool OptionsModel::GetAltFullscreen()
{
	return ui::Engine::Ref().GetAltFullscreen();
}

void OptionsModel::SetAltFullscreen(bool altFullscreen)
{
	ui::Engine::Ref().SetAltFullscreen(altFullscreen);
	Client::Ref().SetPref("AltFullscreen", altFullscreen);
	notifySettingsChanged();
}

bool OptionsModel::GetForceIntegerScaling()
{
	return ui::Engine::Ref().GetForceIntegerScaling();
}

void OptionsModel::SetForceIntegerScaling(bool forceIntegerScaling)
{
	ui::Engine::Ref().SetForceIntegerScaling(forceIntegerScaling);
	Client::Ref().SetPref("ForceIntegerScaling", forceIntegerScaling);
	notifySettingsChanged();
}

bool OptionsModel::GetFastQuit()
{
	return ui::Engine::Ref().GetFastQuit();
}
void OptionsModel::SetFastQuit(bool fastquit)
{
	ui::Engine::Ref().SetFastQuit(fastquit);
	Client::Ref().SetPref("FastQuit", bool(fastquit));
	notifySettingsChanged();
}

int OptionsModel::GetDecoSpace()
{
	return gModel->GetDecoSpace();
}
void OptionsModel::SetDecoSpace(int decoSpace)
{
	gModel->SetDecoSpace(decoSpace);
	notifySettingsChanged();
}

bool OptionsModel::GetShowAvatars()
{
	return Client::Ref().GetPrefBool("ShowAvatars", true);
}

void OptionsModel::SetShowAvatars(bool state)
{
	Client::Ref().SetPref("ShowAvatars", state);
	notifySettingsChanged();
}

bool OptionsModel::GetMouseClickRequired()
{
	return Client::Ref().GetPrefBool("MouseClickRequired", false);
}

void OptionsModel::SetMouseClickRequired(bool mouseClickRequired)
{
	Client::Ref().SetPref("MouseClickRequired", mouseClickRequired);
	gModel->SetMouseClickRequired(mouseClickRequired);
	notifySettingsChanged();
}

bool OptionsModel::GetIncludePressure()
{
	return Client::Ref().GetPrefBool("Simulation.IncludePressure", true);
}

void OptionsModel::SetIncludePressure(bool includePressure)
{
	Client::Ref().SetPref("Simulation.IncludePressure", includePressure);
	gModel->SetIncludePressure(includePressure);
	notifySettingsChanged();
}

bool OptionsModel::GetPerfectCircle()
{
	return Client::Ref().GetPrefBool("PerfectCircleBrush", true);
}

void OptionsModel::SetPerfectCircle(bool perfectCircle)
{
	Client::Ref().SetPref("PerfectCircleBrush", perfectCircle);
	gModel->SetPerfectCircle(perfectCircle);
	notifySettingsChanged();
}

bool OptionsModel::GetMomentumScroll()
{
	return Client::Ref().GetPrefBool("MomentumScroll", true);
}

void OptionsModel::SetMomentumScroll(bool state)
{
	Client::Ref().SetPref("MomentumScroll", state);
	ui::Engine::Ref().SetMomentumScroll(state);
	notifySettingsChanged();
}

bool OptionsModel::GetAutoDrawLimit()
{
	return Client::Ref().GetPrefBool("AutoDrawLimit", false);
}

void OptionsModel::SetAutoDrawLimit(bool state)
{
	Client::Ref().SetPref("AutoDrawLimit", state);
	notifySettingsChanged();
}

void OptionsModel::notifySettingsChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySettingsChanged(this);
	}
}

OptionsModel::~OptionsModel() {
}

