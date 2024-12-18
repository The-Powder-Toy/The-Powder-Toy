#pragma once
#include <memory>
#include <mutex>
#include <utility>
#include <vector>
#include "common/ExplicitSingleton.h"
#include "graphics/RendererSettings.h"
#include "common/String.h"

class GameSave;
class VideoBuffer;
class Simulation;
class Renderer;

class SaveRenderer: public ExplicitSingleton<SaveRenderer>
{
	std::unique_ptr<Simulation> sim;
	std::unique_ptr<Renderer> ren;
	std::mutex renderMutex;

public:
	SaveRenderer();
	~SaveRenderer();
	std::unique_ptr<VideoBuffer> Render(const GameSave *save, bool fire, RendererSettings rendererSettings);
};
