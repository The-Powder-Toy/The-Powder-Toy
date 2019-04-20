#ifndef RENDERCONTROLLER_H_
#define RENDERCONTROLLER_H_

class ControllerCallback;
class RenderView;
class RenderModel;
class Renderer;
class RenderController
{
	RenderView * renderView;
	RenderModel * renderModel;
	ControllerCallback * callback;
public:
	bool HasExited;
	RenderController(Renderer * ren, ControllerCallback * callback = nullptr);
	void Exit();
	RenderView * GetView() { return renderView; }
	virtual ~RenderController();
	void SetRenderMode(unsigned int renderMode);
	void UnsetRenderMode(unsigned int renderMode);
	void SetDisplayMode(unsigned int renderMode);
	void UnsetDisplayMode(unsigned int renderMode);
	void SetColourMode(unsigned int renderMode);
	void LoadRenderPreset(int presetNum);
};

#endif /* RENDERCONTROLLER_H_ */
