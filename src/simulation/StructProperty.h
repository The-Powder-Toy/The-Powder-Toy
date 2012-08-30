//
//  StructProperty.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef The_Powder_Toy_StructProperty_h
#define The_Powder_Toy_StructProperty_h

#include <string>

struct StructProperty
{
	enum PropertyType { ParticleType, Colour, Integer, UInteger, Float, String, Char, UChar };
	std::string Name;
	PropertyType Type;
	intptr_t Offset;
	
	StructProperty(std::string name, PropertyType type, intptr_t offset):
	Name(name),
	Type(type),
	Offset(offset)
	{
		
	}

	StructProperty():
	Name(""),
	Type(Char),
	Offset(0)
	{
		
	}
};

#endif
