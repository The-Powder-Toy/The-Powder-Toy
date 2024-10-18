#pragma once
#include "common/String.h"
#include <cstdint>
#include <variant>

using PropertyValue = std::variant<
	int,
	unsigned int,
	float
>;

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

	StructProperty();
	StructProperty(ByteString name, PropertyType type, intptr_t offset);

	bool operator ==(const StructProperty &other) const;
	::String ToString(const PropertyValue &value) const;
};

struct StructPropertyAlias
{
	ByteString from, to;
};
