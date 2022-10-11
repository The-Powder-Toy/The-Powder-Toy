#ifndef SAVERENDERER_H_
#define SAVERENDERER_H_
#include "Config.h"
#include "common/Singleton.h"
#include <mutex>

class GameSave;
class VideoBuffer;
class Graphics;
class Simulation;
class Renderer;

class SaveRenderer: public Singleton<SaveRenderer> {
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

#endif /* SAVERENDERER_H_ */
