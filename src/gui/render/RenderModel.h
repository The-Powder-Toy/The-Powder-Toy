#pragma once
#include <vector>
#include <cstdint>

class RenderView;
class Renderer;
struct RendererSettings;
class Simulation;
class RenderModel
{
	std::vector<RenderView*> observers;
	Renderer * renderer = nullptr;
	RendererSettings *rendererSettings = nullptr;
	Simulation *sim = nullptr;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyRenderChanged();
	void notifyDisplayChanged();
	void notifyColourChanged();
public:
	Renderer * GetRenderer();
	RendererSettings *GetRendererSettings();
	Simulation *GetSimulation();
	void AddObserver(RenderView * observer);
	void SetRenderer(Renderer * ren, RendererSettings *newRendererSettings);
	void SetSimulation(Simulation *newSim);
	void SetRenderMode(uint32_t newRenderMode);
	uint32_t GetRenderMode();
	void SetDisplayMode(uint32_t newDisplayMode);
	uint32_t GetDisplayMode();
	void SetColorMode(uint32_t newColorMode);
	uint32_t GetColorMode();
	void LoadRenderPreset(int presetNum);
	virtual ~RenderModel();
};
