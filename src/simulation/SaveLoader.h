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
	static int LoadSave(unsigned char * data, int dataLength, Simulation * sim);
	static unsigned char * BuildSave(int & dataLength, Simulation * sim);
	static int OPSLoadSave(unsigned char * data, int dataLength, Simulation * sim);
	static unsigned char * OPSBuildSave(int & dataLength, Simulation * sim);
	static int PSVLoadSave(unsigned char * data, int dataLength, Simulation * sim);
	static unsigned char * PSVBuildSave(int & dataLength, Simulation * sim);
};

#endif /* SAVELOADER_H_ */
