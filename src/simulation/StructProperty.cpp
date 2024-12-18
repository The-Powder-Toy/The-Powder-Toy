#include "StructProperty.h"
#include <cassert>

StructProperty::StructProperty(ByteString name, PropertyType type, intptr_t offset):
Name(name),
Type(type),
Offset(offset)
{

}

StructProperty::StructProperty():
Name(""),
Type(Integer),
Offset(0)
{

}

bool StructProperty::operator ==(const StructProperty &other) const
{
	return Name == other.Name &&
	       Type == other.Type &&
	       Offset == other.Offset;
}

String StructProperty::ToString(const PropertyValue &value) const
{
	::String str;
	switch (Type)
	{
	case StructProperty::Float:
		str = String::Build(std::get<float>(value));
		break;

	case StructProperty::ParticleType:
	case StructProperty::Integer:
		str = String::Build(std::get<int>(value));
		break;

	case StructProperty::UInteger:
		str = String::Build(std::get<unsigned int>(value));
		break;

	default:
		assert(false);
		break;
	}
	return str;
}
