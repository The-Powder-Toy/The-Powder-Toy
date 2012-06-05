//
//  Particle.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef The_Powder_Toy_Particle_h
#define The_Powder_Toy_Particle_h

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
	static std::vector<StructProperty> GetProperties()
	{
		std::vector<StructProperty> properties;
		properties.push_back(StructProperty("type", StructProperty::ParticleType, offsetof(Particle, type)));
		properties.push_back(StructProperty("life", StructProperty::ParticleType, offsetof(Particle, life)));
		properties.push_back(StructProperty("ctype", StructProperty::ParticleType, offsetof(Particle, ctype)));
		properties.push_back(StructProperty("x", StructProperty::Float, offsetof(Particle, x)));
		properties.push_back(StructProperty("y", StructProperty::Float, offsetof(Particle, y)));
		properties.push_back(StructProperty("vx", StructProperty::Float, offsetof(Particle, vx)));
		properties.push_back(StructProperty("vy", StructProperty::Float, offsetof(Particle, vy)));
		properties.push_back(StructProperty("temp", StructProperty::Float, offsetof(Particle, temp)));
		properties.push_back(StructProperty("flags", StructProperty::UInteger, offsetof(Particle, flags)));
		properties.push_back(StructProperty("tmp", StructProperty::Integer, offsetof(Particle, tmp)));
		properties.push_back(StructProperty("tmp2", StructProperty::Integer, offsetof(Particle, tmp2)));
		properties.push_back(StructProperty("dcolour", StructProperty::UInteger, offsetof(Particle, dcolour)));
		return properties;
	}
};

#endif
