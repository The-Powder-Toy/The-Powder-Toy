#include "RenderModel.h"
#include "RenderView.h"
#include "gui/game/RenderPreset.h"
#include "gui/game/GameController.h"
#include "gui/game/GameView.h"
#include "graphics/Renderer.h"

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
	rendererSettings->renderMode = newRenderMode;
	notifyRenderChanged();
}

uint32_t RenderModel::GetRenderMode()
{
	return rendererSettings->renderMode;
}

void RenderModel::SetDisplayMode(uint32_t newDisplayMode)
{
	rendererSettings->displayMode = newDisplayMode;
	notifyDisplayChanged();
}

uint32_t RenderModel::GetDisplayMode()
{
	return rendererSettings->displayMode;
}

void RenderModel::SetColorMode(uint32_t newColorMode)
{
	rendererSettings->colorMode = newColorMode;
	notifyColourChanged();
}

uint32_t RenderModel::GetColorMode()
{
	return rendererSettings->colorMode;
}

void RenderModel::LoadRenderPreset(int presetNum)
{
	RenderPreset preset = Renderer::renderModePresets[presetNum];
	SetRenderMode(preset.renderMode);
	SetDisplayMode(preset.displayMode);
	SetColorMode(preset.colorMode);
}

void RenderModel::SetRenderer(Renderer * ren, RendererSettings *newRendererSettings)
{
	renderer = ren;
	rendererSettings = newRendererSettings;
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

RendererSettings *RenderModel::GetRendererSettings()
{
	return rendererSettings;
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
