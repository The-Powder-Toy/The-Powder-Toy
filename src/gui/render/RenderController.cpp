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

void RenderController::SetRenderMode(unsigned int renderMode)
{
	renderModel->SetRenderMode(renderMode);
}

void RenderController::UnsetRenderMode(unsigned int renderMode)
{
	renderModel->UnsetRenderMode(renderMode);
}

void RenderController::SetDisplayMode(unsigned int renderMode)
{
	renderModel->SetDisplayMode(renderMode);
}

void RenderController::UnsetDisplayMode(unsigned int renderMode)
{
	renderModel->UnsetDisplayMode(renderMode);
}

void RenderController::SetColourMode(unsigned int renderMode)
{
	renderModel->SetColourMode(renderMode);
}

void RenderController::LoadRenderPreset(int presetNum)
{
	renderModel->LoadRenderPreset(presetNum);
}

void RenderController::Exit()
{
	renderView->CloseActiveWindow();
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}

RenderController::~RenderController()
{
	renderView->CloseActiveWindow();
	delete callback;
	delete renderModel;
	delete renderView;
}

