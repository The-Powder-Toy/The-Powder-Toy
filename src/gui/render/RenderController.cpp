#include "RenderController.h"

#include "RenderView.h"
#include "RenderModel.h"

#include "Controller.h"

RenderController::RenderController(Renderer * ren, std::function<void ()> onDone_):
	HasExited(false)
{
	renderView = new RenderView();
	renderModel = new RenderModel();

	renderView->AttachController(this);
	renderModel->AddObserver(renderView);

	renderModel->SetRenderer(ren);
	onDone = onDone_;
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
	if (onDone)
		onDone();
	HasExited = true;
}

RenderController::~RenderController()
{
	renderView->CloseActiveWindow();
	delete renderModel;
	delete renderView;
}

