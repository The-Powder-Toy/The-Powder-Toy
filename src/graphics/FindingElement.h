#pragma once
#include "simulation/StructProperty.h"

struct FindingElement
{
	StructProperty property;
	PropertyValue value;

	bool operator ==(const FindingElement &other) const
	{
		return property == other.property &&
		       value == other.value;
	}
};
