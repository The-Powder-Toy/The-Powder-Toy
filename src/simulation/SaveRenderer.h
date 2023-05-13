#pragma once
#include <memory>
#include <mutex>
#include <utility>
#include <vector>
#include "common/ExplicitSingleton.h"
#include "common/String.h"

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
	std::pair<std::unique_ptr<VideoBuffer>, std::vector<ByteString>> Render(const GameSave *save, bool decorations = true, bool fire = true, Renderer *renderModeSource = nullptr);
	void Flush(int begin, int end);
	virtual ~SaveRenderer();
};
