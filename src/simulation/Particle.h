#pragma once
#include "StructProperty.h"
#include <vector>

struct Particle
{
	int type;
	int life, ctype;
	float x, y, vx, vy;
	float temp;
	int tmp3;
	int tmp4;
	int flags;
	int tmp;
	int tmp2;
	unsigned int dcolour;
	/** Returns a list of properties, their type and offset within the structure that can be changed
	 by higher-level processes referring to them by name such as Lua or the property tool **/
	static std::vector<StructProperty> const &GetProperties();
	static std::vector<StructPropertyAlias> const &GetPropertyAliases();
};
