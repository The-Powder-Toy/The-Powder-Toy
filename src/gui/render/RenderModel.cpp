#include "RenderModel.h"

#include "RenderView.h"

#include "gui/game/RenderPreset.h"

#include "graphics/Renderer.h"

RenderModel::RenderModel():
	renderer(NULL)
{

}

void RenderModel::AddObserver(RenderView * observer)
{
	observers.push_back(observer);
	observer->NotifyRendererChanged(this);
	observer->NotifyRenderChanged(this);
	observer->NotifyDisplayChanged(this);
	observer->NotifyColourChanged(this);
}

void RenderModel::SetRenderMode(uint32_t newRenderMode)
{
	if (renderer)
	{
		renderer->SetRenderMode(newRenderMode);
	}
	notifyRenderChanged();
}

uint32_t RenderModel::GetRenderMode()
{
	return renderer ? renderer->GetRenderMode() : 0;
}

void RenderModel::SetDisplayMode(uint32_t newDisplayMode)
{
	if (renderer)
	{
		renderer->SetDisplayMode(newDisplayMode);
	}
	notifyDisplayChanged();
}

uint32_t RenderModel::GetDisplayMode()
{
	return renderer ? renderer->GetDisplayMode() : 0;
}

void RenderModel::SetColorMode(uint32_t newColorMode)
{
	if (renderer)
	{
		renderer->SetColorMode(newColorMode);
	}
	notifyColourChanged();
}

uint32_t RenderModel::GetColorMode()
{
	return renderer ? renderer->GetColorMode() : 0;
}

void RenderModel::LoadRenderPreset(int presetNum)
{
	RenderPreset preset = renderer->renderModePresets[presetNum];
	SetRenderMode(preset.renderMode);
	SetDisplayMode(preset.displayMode);
	SetColorMode(preset.colorMode);
}

void RenderModel::SetRenderer(Renderer * ren)
{
	renderer = ren;
	notifyRendererChanged();
	notifyRenderChanged();
	notifyDisplayChanged();
	notifyColourChanged();
}

void RenderModel::SetSimulation(Simulation *newSim)
{
	sim = newSim;
	notifySimulationChanged();
	notifyRenderChanged();
	notifyDisplayChanged();
	notifyColourChanged();
}

Renderer * RenderModel::GetRenderer()
{
	return renderer;
}

Simulation *RenderModel::GetSimulation()
{
	return sim;
}

void RenderModel::notifyRendererChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
	}
}

void RenderModel::notifySimulationChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySimulationChanged(this);
	}
}

void RenderModel::notifyRenderChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRenderChanged(this);
	}
}

void RenderModel::notifyDisplayChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyDisplayChanged(this);
	}
}

void RenderModel::notifyColourChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyColourChanged(this);
	}
}

RenderModel::~RenderModel() {
}
