/*
 * SaveRenderer.h
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#ifndef SAVERENDERER_H_
#define SAVERENDERER_H_
#ifdef OGLI
#include "graphics/OpenGLHeaders.h" 
#endif
#include "Singleton.h"

class GameSave;
class Thumbnail;
class Graphics;
class Simulation;
class Renderer;

class SaveRenderer: public Singleton<SaveRenderer> {
	Graphics * g;
	Simulation * sim;
	Renderer * ren;
public:
	SaveRenderer();
	Thumbnail * Render(GameSave * save, bool decorations = true);
	Thumbnail * Render(unsigned char * saveData, int saveDataSize, bool decorations = true);
	virtual ~SaveRenderer();

private:
#if defined(OGLR) || defined(OGLI)
	GLuint fboTex, fbo;
#endif
};

#endif /* SAVERENDERER_H_ */
