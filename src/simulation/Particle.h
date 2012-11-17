//
//  Particle.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef The_Powder_Toy_Particle_h
#define The_Powder_Toy_Particle_h

#include <vector>
#include "StructProperty.h"

struct Particle
{
	int type;
	int life, ctype;
	float x, y, vx, vy;
	float temp;
	float pavg[2];
	int flags;
	int tmp;
	int tmp2;
	unsigned int dcolour;
	/** Returns a list of properties, their type and offset within the structure that can be changed
	 by higher-level processes refering to them by name such as Lua or the property tool **/
	static std::vector<StructProperty> GetProperties();
};

#endif
