#include "OptionsController.h"
#include "gui/dialogues/ErrorMessage.h"

OptionsController::OptionsController(GameModel * gModel_, ControllerCallback * callback_):
	callback(callback_),
	gModel(gModel_),
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

void OptionsController::SetEdgeMode(int airMode)
{
	model->SetEdgeMode(airMode);
}

void OptionsController::SetFullscreen(bool fullscreen)
{
	model->SetFullscreen(fullscreen);
}

void OptionsController::SetShowAvatars(bool showAvatars)
{
	model->SetShowAvatars(showAvatars);
}

void OptionsController::SetScale(bool scale)
{
	if(scale)
	{
		if(ui::Engine::Ref().GetMaxWidth() >= ui::Engine::Ref().GetWidth() * 2 && ui::Engine::Ref().GetMaxHeight() >= ui::Engine::Ref().GetHeight() * 2)
			model->SetScale(scale);
		else
		{
			new ErrorMessage("Screen resolution error", "Your screen size is too small to use this scale mode.");
			model->SetScale(false);
		}
	}
	else
		model->SetScale(scale);

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
	if(ui::Engine::Ref().GetWindow() == view)
	{
		ui::Engine::Ref().CloseWindow();
	}
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}


OptionsController::~OptionsController() {
	if(ui::Engine::Ref().GetWindow() == view)
	{
		ui::Engine::Ref().CloseWindow();
	}
	delete model;
	delete view;
	if(callback)
		delete callback;
}

