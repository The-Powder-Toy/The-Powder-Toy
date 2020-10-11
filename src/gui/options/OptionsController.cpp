#include "OptionsController.h"

#include "OptionsView.h"
#include "OptionsModel.h"

#include "Controller.h"

OptionsController::OptionsController(GameModel * gModel_, std::function<void ()> onDone_):
	gModel(gModel_),
	onDone(onDone_),
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

void OptionsController::SetForceIntegerScaling(bool forceIntegerScaling)
{
	model->SetForceIntegerScaling(forceIntegerScaling);
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

void OptionsController::SetDecoSpace(int decoSpace)
{
	model->SetDecoSpace(decoSpace);
}

OptionsView * OptionsController::GetView()
{
	return view;
}

void OptionsController::SetMouseClickrequired(bool mouseClickRequired)
{
	model->SetMouseClickRequired(mouseClickRequired);
}

void OptionsController::SetIncludePressure(bool includePressure)
{
	model->SetIncludePressure(includePressure);
}

void OptionsController::SetPerfectCircle(bool perfectCircle)
{
	model->SetPerfectCircle(perfectCircle);
}

void OptionsController::SetMomentumScroll(bool momentumScroll)
{
	model->SetMomentumScroll(momentumScroll);
}

void OptionsController::SetAutoDrawLimit(bool autoDrawLimit)
{
	model->SetAutoDrawLimit(autoDrawLimit);
}

void OptionsController::Exit()
{
	view->CloseActiveWindow();

	if (onDone)
		onDone();
	HasExited = true;
}


OptionsController::~OptionsController()
{
	view->CloseActiveWindow();
	delete model;
	delete view;
}

