#ifndef STRUCTPROPERTY_H_
#define STRUCTPROPERTY_H_

#include "common/String.h"
#include <cstdint>

struct StructProperty
{
	enum PropertyType
	{
		TransitionType,
		ParticleType,
		Colour,
		Integer,
		UInteger,
		Float,
		BString,
		String,
		UChar,
		Removed
	};
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
	Type(Integer),
	Offset(0)
	{

	}
};

union PropertyValue {
	int Integer;
	unsigned int UInteger;
	float Float;
};

struct StructPropertyAlias
{
	ByteString from, to;
};

#endif
