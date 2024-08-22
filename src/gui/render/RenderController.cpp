#include "RenderController.h"

#include "RenderView.h"
#include "RenderModel.h"

#include "Controller.h"

RenderController::RenderController(Simulation *sim, Renderer * ren, RendererSettings *rendererSettings, std::function<void ()> onDone_):
	HasExited(false)
{
	renderView = new RenderView();
	renderModel = new RenderModel();

	renderModel->SetRenderer(ren, rendererSettings);

	renderView->AttachController(this);
	renderModel->AddObserver(renderView);

	renderModel->SetSimulation(sim);

	onDone = onDone_;
}

void RenderController::SetRenderMode(uint32_t newRenderMode)
{
	renderModel->SetRenderMode(newRenderMode);
}

uint32_t RenderController::GetRenderMode()
{
	return renderModel->GetRenderMode();
}

void RenderController::SetDisplayMode(uint32_t newDisplayMode)
{
	renderModel->SetDisplayMode(newDisplayMode);
}

uint32_t RenderController::GetDisplayMode()
{
	return renderModel->GetDisplayMode();
}

void RenderController::SetColorMode(uint32_t newColorMode)
{
	renderModel->SetColorMode(newColorMode);
}

uint32_t RenderController::GetColorMode()
{
	return renderModel->GetColorMode();
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
	delete renderModel;
	if (renderView->CloseActiveWindow())
	{
		delete renderView;
	}
}

