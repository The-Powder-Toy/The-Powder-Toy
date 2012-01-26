/*
 * RenderController.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#include "RenderController.h"

RenderController::RenderController(Renderer * ren, ControllerCallback * callback):
	HasExited(false)
{
	renderView = new RenderView();
	renderModel = new RenderModel();

	renderView->AttachController(this);
	renderModel->AddObserver(renderView);

	renderModel->SetRenderer(ren);
	this->callback = callback;
}

void RenderController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == renderView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}

RenderController::~RenderController() {
	delete renderView;
	delete renderModel;
}

