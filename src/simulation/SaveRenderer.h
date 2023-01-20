#pragma once
#include "common/ExplicitSingleton.h"
#include <mutex>

class GameSave;
class VideoBuffer;
class Graphics;
class Simulation;
class Renderer;

class SaveRenderer: public ExplicitSingleton<SaveRenderer> {
	Graphics * g;
	Simulation * sim;
	Renderer * ren;
	std::mutex renderMutex;
public:
	SaveRenderer();
	VideoBuffer * Render(GameSave * save, bool decorations = true, bool fire = true, Renderer *renderModeSource = nullptr);
	VideoBuffer * Render(unsigned char * saveData, int saveDataSize, bool decorations = true, bool fire = true);
	void Flush(int begin, int end);
	virtual ~SaveRenderer();
};
