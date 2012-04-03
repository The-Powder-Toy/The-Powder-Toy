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
	static int Info(unsigned char * data, int dataLength, int & width, int & height);
	static int Load(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x, int y);
	static unsigned char * Build(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h);
private:
	static int OPSInfo(unsigned char * data, int dataLength, int & width, int & height);
	static int OPSLoad(unsigned char * data, int dataLength, Simulation * sim);
	static unsigned char * OPSBuild(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h);
	static int PSVInfo(unsigned char * data, int dataLength, int & width, int & height);
	static int PSVLoad(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x, int y);
	static unsigned char * PSVBuild(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h);
};

#endif /* SAVELOADER_H_ */
