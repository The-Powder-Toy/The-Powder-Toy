#include "RenderModel.h"
#include "gui/game/RenderPreset.h"

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

void RenderModel::SetRenderMode(unsigned int renderMode)
{
	if(renderer)
		renderer->AddRenderMode(renderMode);
	notifyRenderChanged();
}

void RenderModel::UnsetRenderMode(unsigned int renderMode)
{
	if(renderer)
		renderer->RemoveRenderMode(renderMode);
	notifyRenderChanged();
}

unsigned int RenderModel::GetRenderMode()
{
	if(renderer)
		return renderer->render_mode;
	else
		return 0;
}

void RenderModel::SetDisplayMode(unsigned int displayMode)
{
	if(renderer)
		renderer->AddDisplayMode(displayMode);
	notifyDisplayChanged();
}

void RenderModel::UnsetDisplayMode(unsigned int displayMode)
{
	if(renderer)
		renderer->RemoveDisplayMode(displayMode);
	notifyDisplayChanged();
}

unsigned int RenderModel::GetDisplayMode()
{
	if(renderer)
		return renderer->display_mode;
	else
		return 0;
}

void RenderModel::SetColourMode(unsigned int colourMode)
{
	if(renderer)
		renderer->SetColourMode(colourMode);
	notifyColourChanged();
}

unsigned int RenderModel::GetColourMode()
{
	if(renderer)
		return renderer->colour_mode;
	else
		return 0;
}

void RenderModel::LoadRenderPreset(int presetNum)
{
	RenderPreset preset = renderer->renderModePresets[presetNum];
	renderer->SetRenderMode(preset.RenderModes);
	renderer->SetDisplayMode(preset.DisplayModes);
	renderer->SetColourMode(preset.ColourMode);
	notifyRenderChanged();
	notifyDisplayChanged();
	notifyColourChanged();
}

void RenderModel::SetRenderer(Renderer * ren)
{
	renderer = ren;
	notifyRendererChanged();
	notifyRenderChanged();
	notifyDisplayChanged();
	notifyColourChanged();
}

Renderer * RenderModel::GetRenderer()
{
	return renderer;
}

void RenderModel::notifyRendererChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
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
