#pragma once
#include "StructProperty.h"
#include "common/String.h"
#include <stdexcept>

class Simulation;

struct AccessProperty
{
	int propertyIndex;
	PropertyValue propertyValue;

	void Set(Simulation *sim, int i) const;
	PropertyValue Get(const Simulation *sim, int i) const;

	struct ParseError : public std::runtime_error
	{
		using runtime_error::runtime_error;
	};
	static AccessProperty Parse(int prop, String value);
};

