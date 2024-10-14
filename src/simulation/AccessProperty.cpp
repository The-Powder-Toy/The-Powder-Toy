#include "AccessProperty.h"
#include "simulation/Simulation.h"

void AccessProperty::Set(Simulation *sim, int i) const
{
	auto &part = sim->parts[i];
	auto &prop = Particle::GetProperties()[propertyIndex];
	auto *ptr = (reinterpret_cast<char *>(&part) + prop.Offset);
	if (propertyIndex == FIELD_TYPE)
	{
		sim->part_change_type(i, int(part.x + 0.5f), int(part.y + 0.5f), std::get<int>(propertyValue));
		return;
	}
	switch (prop.Type)
	{
	case StructProperty::Float:
		*reinterpret_cast<float *>(ptr) = std::get<float>(propertyValue);
		break;

	case StructProperty::ParticleType:
	case StructProperty::Integer:
		*reinterpret_cast<int *>(ptr) = std::get<int>(propertyValue);
		break;

	case StructProperty::UInteger:
		*reinterpret_cast<unsigned int *>(ptr) = std::get<unsigned int>(propertyValue);
		break;

	default:
		break;
	}
}

PropertyValue AccessProperty::Get(const Simulation *sim, int i) const
{
	PropertyValue propValue;
	auto &part = sim->parts[i];
	auto &prop = Particle::GetProperties()[propertyIndex];
	auto *ptr = (reinterpret_cast<const char *>(&part) + prop.Offset);
	switch (prop.Type)
	{
	case StructProperty::Float:
		propValue = *reinterpret_cast<const float *>(ptr);
		break;

	case StructProperty::ParticleType:
	case StructProperty::Integer:
		propValue = *reinterpret_cast<const int *>(ptr);
		break;

	case StructProperty::UInteger:
		propValue = *reinterpret_cast<const unsigned int *>(ptr);
		break;

	default:
		break;
	}
	return propValue;
}
