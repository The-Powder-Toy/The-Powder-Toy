#pragma once
#include <functional>
#include <cstdint>

class RenderView;
class RenderModel;
class Renderer;
struct RendererSettings;
class Simulation;
class RenderController
{
	RenderView * renderView;
	RenderModel * renderModel;
	std::function<void ()> onDone;
public:
	bool HasExited;
	RenderController(Simulation *sim, Renderer * ren, RendererSettings *rendererSettings, std::function<void ()> onDone = nullptr);
	void Exit();
	RenderView * GetView() { return renderView; }
	virtual ~RenderController();
	void SetRenderMode(uint32_t newRenderMode);
	uint32_t GetRenderMode();
	void SetDisplayMode(uint32_t newDisplayMode);
	uint32_t GetDisplayMode();
	void SetColorMode(uint32_t newColorMode);
	uint32_t GetColorMode();
	void LoadRenderPreset(int presetNum);
};
