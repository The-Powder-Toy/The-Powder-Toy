#ifndef STRUCTPROPERTY_H_
#define STRUCTPROPERTY_H_

#include "common/String.h"
#include <cstdint>

struct StructProperty
{
	enum PropertyType { ParticleType, Colour, Integer, UInteger, Float, BString, String, Char, UChar, Removed };
	ByteString Name;
	PropertyType Type;
	intptr_t Offset;

	StructProperty(ByteString name, PropertyType type, intptr_t offset):
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
