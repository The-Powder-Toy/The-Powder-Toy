#pragma once
#include <vector>

class RenderView;
class Renderer;
class Simulation;
class RenderModel
{
	std::vector<RenderView*> observers;
	Renderer * renderer;
	Simulation *sim = nullptr;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyRenderChanged();
	void notifyDisplayChanged();
	void notifyColourChanged();
public:
	RenderModel();
	Renderer * GetRenderer();
	Simulation *GetSimulation();
	void AddObserver(RenderView * observer);
	void SetRenderer(Renderer * ren);
	void SetSimulation(Simulation *newSim);
	void SetRenderMode(unsigned int renderMode);
	void UnsetRenderMode(unsigned int renderMode);
	unsigned int GetRenderMode();
	void SetDisplayMode(unsigned int displayMode);
	void UnsetDisplayMode(unsigned int displayMode);
	unsigned int GetDisplayMode();
	void SetColourMode(unsigned int colourMode);
	unsigned int GetColourMode();
	void LoadRenderPreset(int presetNum);
	virtual ~RenderModel();
};
