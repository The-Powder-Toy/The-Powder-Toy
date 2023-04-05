#pragma once
#include <memory>
#include <mutex>
#include "common/ExplicitSingleton.h"

class GameSave;
class VideoBuffer;
class Graphics;
class Simulation;
class Renderer;

class SaveRenderer: public ExplicitSingleton<SaveRenderer> {
	Simulation * sim;
	Renderer * ren;
	std::mutex renderMutex;
public:
	SaveRenderer();
	std::unique_ptr<VideoBuffer> Render(GameSave * save, bool decorations = true, bool fire = true, Renderer *renderModeSource = nullptr);
	void Flush(int begin, int end);
	virtual ~SaveRenderer();
};
