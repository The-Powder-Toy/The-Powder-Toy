#pragma once
#include "common/String.h"
#include <cstdint>
#include <variant>

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

	bool operator ==(const StructProperty &other) const
	{
		return Name == other.Name &&
		       Type == other.Type &&
		       Offset == other.Offset;
	}
};

using PropertyValue = std::variant<
	int,
	unsigned int,
	float
>;

struct StructPropertyAlias
{
	ByteString from, to;
};
