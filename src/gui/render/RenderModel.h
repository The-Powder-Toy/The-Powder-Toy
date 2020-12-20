#ifndef RENDERMODEL_H_
#define RENDERMODEL_H_
#include "Config.h"

#include <vector>

class RenderView;
class Renderer;
class RenderModel
{
	std::vector<RenderView*> observers;
	Renderer * renderer;
	void notifyRendererChanged();
	void notifyRenderChanged();
	void notifyDisplayChanged();
	void notifyColourChanged();
public:
	RenderModel();
	Renderer * GetRenderer();
	void AddObserver(RenderView * observer);
	void SetRenderer(Renderer * ren);
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

#endif /* RENDERMODEL_H_ */
