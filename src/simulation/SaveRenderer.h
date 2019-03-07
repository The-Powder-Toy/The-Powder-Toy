#ifndef SAVERENDERER_H_
#define SAVERENDERER_H_
#ifdef OGLI
#include "graphics/OpenGLHeaders.h"
#endif
#include "common/Singleton.h"
#include "common/tpt-thread.h"

class GameSave;
class VideoBuffer;
class Graphics;
class Simulation;
class Renderer;

class SaveRenderer: public Singleton<SaveRenderer> {
	Graphics * g;
	Simulation * sim;
	Renderer * ren;
	pthread_mutex_t renderMutex;
public:
	SaveRenderer();
	VideoBuffer * Render(GameSave * save, bool decorations = true, bool fire = true);
	VideoBuffer * Render(unsigned char * saveData, int saveDataSize, bool decorations = true, bool fire = true);
	virtual ~SaveRenderer();

private:
#if defined(OGLR) || defined(OGLI)
	GLuint fboTex, fbo;
#endif
};

#endif /* SAVERENDERER_H_ */
