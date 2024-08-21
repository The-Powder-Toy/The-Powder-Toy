#include "OptionsModel.h"
#include "OptionsView.h"
#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "simulation/gravity/Gravity.h"
#include "prefs/GlobalPrefs.h"
#include "common/clipboard/Clipboard.h"
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
	return bool(sim->grav);
}

void OptionsModel::SetNewtonianGravity(bool state)
{
	sim->EnableNewtonianGravity(state);
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
	GlobalPrefs::Ref().Set("Simulation.EdgeMode", edgeMode);
	gModel->SetEdgeMode(edgeMode);
	notifySettingsChanged();
}

int OptionsModel::GetTemperatureScale()
{
	return gModel->GetTemperatureScale();
}
void OptionsModel::SetTemperatureScale(int temperatureScale)
{
	GlobalPrefs::Ref().Set("Renderer.TemperatureScale", temperatureScale);
	gModel->SetTemperatureScale(temperatureScale);
	notifySettingsChanged();
}

int OptionsModel::GetThreadedRendering()
{
	return gModel->GetThreadedRendering();
}

void OptionsModel::SetThreadedRendering(bool newThreadedRendering)
{
	GlobalPrefs::Ref().Set("Renderer.SeparateThread", newThreadedRendering);
	gModel->SetThreadedRendering(newThreadedRendering);
	notifySettingsChanged();
}

float OptionsModel::GetAmbientAirTemperature()
{
	return gModel->GetSimulation()->air->ambientAirTemp;
}
void OptionsModel::SetAmbientAirTemperature(float ambientAirTemp)
{
	GlobalPrefs::Ref().Set("Simulation.AmbientAirTemp", ambientAirTemp);
	gModel->SetAmbientAirTemperature(ambientAirTemp);
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

float OptionsModel::GetCustomGravityX()
{
	return sim->customGravityX;
}

void OptionsModel::SetCustomGravityX(float x)
{
	sim->customGravityX = x;
	notifySettingsChanged();
}

float OptionsModel::GetCustomGravityY()
{
	return sim->customGravityY;
}

void OptionsModel::SetCustomGravityY(float y)
{
	sim->customGravityY = y;
	notifySettingsChanged();
}

int OptionsModel::GetScale()
{
	return ui::Engine::Ref().GetScale();
}

void OptionsModel::SetScale(int scale)
{
	ui::Engine::Ref().SetScale(scale);
	GlobalPrefs::Ref().Set("Scale", int(scale));
	notifySettingsChanged();
}

bool OptionsModel::GetGraveExitsConsole()
{
	return ui::Engine::Ref().GraveExitsConsole;
}

void OptionsModel::SetGraveExitsConsole(bool graveExitsConsole)
{
	ui::Engine::Ref().GraveExitsConsole = graveExitsConsole;
	GlobalPrefs::Ref().Set("GraveExitsConsole", graveExitsConsole);
	notifySettingsChanged();
}

bool OptionsModel::GetNativeClipoard()
{
	return Clipboard::GetEnabled();
}

void OptionsModel::SetNativeClipoard(bool nativeClipoard)
{
	Clipboard::SetEnabled(nativeClipoard);
	GlobalPrefs::Ref().Set("NativeClipboard.Enabled", nativeClipoard);
	notifySettingsChanged();
}

bool OptionsModel::GetResizable()
{
	return ui::Engine::Ref().GetResizable();
}

void OptionsModel::SetResizable(bool resizable)
{
	ui::Engine::Ref().SetResizable(resizable);
	GlobalPrefs::Ref().Set("Resizable", resizable);
	notifySettingsChanged();
}

bool OptionsModel::GetFullscreen()
{
	return ui::Engine::Ref().GetFullscreen();
}
void OptionsModel::SetFullscreen(bool fullscreen)
{
	ui::Engine::Ref().SetFullscreen(fullscreen);
	GlobalPrefs::Ref().Set("Fullscreen", fullscreen);
	notifySettingsChanged();
}

bool OptionsModel::GetChangeResolution()
{
	return ui::Engine::Ref().GetChangeResolution();
}

void OptionsModel::SetChangeResolution(bool newChangeResolution)
{
	ui::Engine::Ref().SetChangeResolution(newChangeResolution);
	GlobalPrefs::Ref().Set("AltFullscreen", newChangeResolution);
	notifySettingsChanged();
}

bool OptionsModel::GetForceIntegerScaling()
{
	return ui::Engine::Ref().GetForceIntegerScaling();
}

void OptionsModel::SetForceIntegerScaling(bool forceIntegerScaling)
{
	ui::Engine::Ref().SetForceIntegerScaling(forceIntegerScaling);
	GlobalPrefs::Ref().Set("ForceIntegerScaling", forceIntegerScaling);
	notifySettingsChanged();
}

bool OptionsModel::GetBlurryScaling()
{
	return ui::Engine::Ref().GetBlurryScaling();
}

void OptionsModel::SetBlurryScaling(bool newBlurryScaling)
{
	ui::Engine::Ref().SetBlurryScaling(newBlurryScaling);
	GlobalPrefs::Ref().Set("BlurryScaling", newBlurryScaling);
	notifySettingsChanged();
}

bool OptionsModel::GetFastQuit()
{
	return ui::Engine::Ref().GetFastQuit();
}
void OptionsModel::SetFastQuit(bool fastquit)
{
	ui::Engine::Ref().SetFastQuit(fastquit);
	GlobalPrefs::Ref().Set("FastQuit", bool(fastquit));
	notifySettingsChanged();
}

int OptionsModel::GetDecoSpace()
{
	return gModel->GetDecoSpace();
}
void OptionsModel::SetDecoSpace(int decoSpace)
{
	GlobalPrefs::Ref().Set("Simulation.DecoSpace", decoSpace);
	gModel->SetDecoSpace(decoSpace);
	notifySettingsChanged();
}

bool OptionsModel::GetShowAvatars()
{
	return ui::Engine::Ref().ShowAvatars;
}

void OptionsModel::SetShowAvatars(bool state)
{
	ui::Engine::Ref().ShowAvatars = state;
	GlobalPrefs::Ref().Set("ShowAvatars", state);
	notifySettingsChanged();
}

bool OptionsModel::GetMouseClickRequired()
{
	return gModel->GetMouseClickRequired();
}

void OptionsModel::SetMouseClickRequired(bool mouseClickRequired)
{
	GlobalPrefs::Ref().Set("MouseClickRequired", mouseClickRequired);
	gModel->SetMouseClickRequired(mouseClickRequired);
	notifySettingsChanged();
}

bool OptionsModel::GetIncludePressure()
{
	return gModel->GetIncludePressure();
}

void OptionsModel::SetIncludePressure(bool includePressure)
{
	GlobalPrefs::Ref().Set("Simulation.IncludePressure", includePressure);
	gModel->SetIncludePressure(includePressure);
	notifySettingsChanged();
}

bool OptionsModel::GetPerfectCircle()
{
	return gModel->GetPerfectCircle();
}

void OptionsModel::SetPerfectCircle(bool perfectCircle)
{
	GlobalPrefs::Ref().Set("PerfectCircleBrush", perfectCircle);
	gModel->SetPerfectCircle(perfectCircle);
	notifySettingsChanged();
}

bool OptionsModel::GetMomentumScroll()
{
	return ui::Engine::Ref().MomentumScroll;
}

void OptionsModel::SetMomentumScroll(bool state)
{
	GlobalPrefs::Ref().Set("MomentumScroll", state);
	ui::Engine::Ref().MomentumScroll = state;
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

