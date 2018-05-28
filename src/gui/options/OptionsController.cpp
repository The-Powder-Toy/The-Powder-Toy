#include "OptionsController.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Engine.h"
#include "gui/game/GameModel.h"

OptionsController::OptionsController(GameModel * gModel_, ControllerCallback * callback_):
	gModel(gModel_),
	callback(callback_),
	HasExited(false)
{
	view = new OptionsView();
	model = new OptionsModel(gModel);
	model->AddObserver(view);

	view->AttachController(this);
}

void OptionsController::SetHeatSimulation(bool state)
{
	model->SetHeatSimulation(state);
}

void OptionsController::SetAmbientHeatSimulation(bool state)
{
	model->SetAmbientHeatSimulation(state);
}

void OptionsController::SetNewtonianGravity(bool state)
{
	model->SetNewtonianGravity(state);
}

void OptionsController::SetWaterEqualisation(bool state)
{
	model->SetWaterEqualisation(state);
}

void OptionsController::SetGravityMode(int gravityMode)
{
	model->SetGravityMode(gravityMode);
}

void OptionsController::SetAirMode(int airMode)
{
	model->SetAirMode(airMode);
}

void OptionsController::SetEdgeMode(int edgeMode)
{
	model->SetEdgeMode(edgeMode);
}

void OptionsController::SetFullscreen(bool fullscreen)
{
	model->SetFullscreen(fullscreen);
}

void OptionsController::SetAltFullscreen(bool altFullscreen)
{
	model->SetAltFullscreen(altFullscreen);
}

void OptionsController::SetShowAvatars(bool showAvatars)
{
	model->SetShowAvatars(showAvatars);
}

void OptionsController::SetScale(int scale)
{
	model->SetScale(scale);
}

void OptionsController::SetResizable(bool resizable)
{
	model->SetResizable(resizable);
}

void OptionsController::SetFastQuit(bool fastquit)
{
	model->SetFastQuit(fastquit);
}

OptionsView * OptionsController::GetView()
{
	return view;
}

void OptionsController::Exit()
{
	view->CloseActiveWindow();

	if (callback)
		callback->ControllerExit();
	HasExited = true;
}


OptionsController::~OptionsController()
{
	view->CloseActiveWindow();
	delete model;
	delete view;
	delete callback;
}

