#ifndef STRUCTPROPERTY_H_
#define STRUCTPROPERTY_H_

#include <string>
#include <stdint.h>

struct StructProperty
{
	enum PropertyType { ParticleType, Colour, Integer, UInteger, Float, String, Char, UChar, Removed };
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

union PropertyValue {
	int Integer;
	unsigned int UInteger;
	float Float;
};

#endif
