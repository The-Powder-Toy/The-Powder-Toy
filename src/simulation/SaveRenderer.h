/*
 * SaveRenderer.h
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#ifndef SAVERENDERER_H_
#define SAVERENDERER_H_

#include "Singleton.h"
#include "search/Thumbnail.h"

class Graphics;
class Simulation;
class Renderer;
class SaveRenderer: public Singleton<SaveRenderer> {
	Graphics * g;
	Simulation * sim;
	Renderer * ren;
public:
	SaveRenderer();
	Thumbnail * Render(unsigned char * data, int dataLength);
	virtual ~SaveRenderer();
};

#endif /* SAVERENDERER_H_ */
