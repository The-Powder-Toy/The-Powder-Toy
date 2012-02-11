/*
 * SaveLoader.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#ifndef SAVELOADER_H_
#define SAVELOADER_H_

#include "Simulation.h"

class SaveLoader {
public:
	static int LoadSave(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x, int y);
	static unsigned char * BuildSave(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h);
	static int OPSLoadSave(unsigned char * data, int dataLength, Simulation * sim);
	static unsigned char * OPSBuildSave(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h);
	static int PSVLoadSave(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x, int y);
	static unsigned char * PSVBuildSave(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h);
};

#endif /* SAVELOADER_H_ */
