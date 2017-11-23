#include "OptionsController.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Engine.h"
#include "gui/game/GameModel.h"

OptionsController::OptionsController(GameModel * gModel_, ControllerCallback * callback_):
	gModel(gModel_),
	callback(callback_),
	HasExited(false)
{
	this->depth3d = ui::Engine::Ref().Get3dDepth();
	this->newScale = ui::Engine::Ref().GetScale();
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

void OptionsController::SetShowAvatars(bool showAvatars)
{
	model->SetShowAvatars(showAvatars);
}

void OptionsController::SetScale(int scale)
{
	newScale = scale;
}

void OptionsController::SetFastQuit(bool fastquit)
{
	model->SetFastQuit(fastquit);
}

void OptionsController::Set3dDepth(int depth)
{
	depth3d = depth;
}

OptionsView * OptionsController::GetView()
{
	return view;
}

void OptionsController::Exit()
{
	view->CloseActiveWindow();
	// only update on close, it would be hard to edit if the changes were live
	ui::Engine::Ref().Set3dDepth(depth3d);

	{
		if (newScale < 1)
			newScale = 1;
		bool reduced_scale = false;
		while (!(ui::Engine::Ref().GetMaxWidth() >= ui::Engine::Ref().GetWidth() * newScale && ui::Engine::Ref().GetMaxHeight() >= ui::Engine::Ref().GetHeight() * newScale) && newScale > 1)
		{
			newScale -= 1;
			reduced_scale = true;
		}
		if (reduced_scale)
			new ErrorMessage("Screen resolution error", "Your screen size is too small to use this scale mode. Using largest available scale.");
		ui::Engine::Ref().SetScale(newScale);
		Client::Ref().SetPref("Scale", newScale);
	}

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

