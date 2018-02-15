#include <cstddef>
#include "Particle.h"

std::vector<StructProperty> Particle::GetProperties()
{
	std::vector<StructProperty> properties;
	properties.emplace_back("type", StructProperty::ParticleType, offsetof(Particle, type));
	properties.emplace_back("life", StructProperty::ParticleType, offsetof(Particle, life));
	properties.emplace_back("ctype", StructProperty::ParticleType, offsetof(Particle, ctype));
	properties.emplace_back("x", StructProperty::Float, offsetof(Particle, x));
	properties.emplace_back("y", StructProperty::Float, offsetof(Particle, y));
	properties.emplace_back("vx", StructProperty::Float, offsetof(Particle, vx));
	properties.emplace_back("vy", StructProperty::Float, offsetof(Particle, vy));
	properties.emplace_back("temp", StructProperty::Float, offsetof(Particle, temp));
	properties.emplace_back("flags", StructProperty::UInteger, offsetof(Particle, flags));
	properties.emplace_back("tmp", StructProperty::Integer, offsetof(Particle, tmp));
	properties.emplace_back("tmp2", StructProperty::Integer, offsetof(Particle, tmp2));
	properties.emplace_back("dcolour", StructProperty::UInteger, offsetof(Particle, dcolour));
	properties.emplace_back("pavg0", StructProperty::Float, offsetof(Particle, pavg[0]));
	properties.emplace_back("pavg1", StructProperty::Float, offsetof(Particle, pavg[1]));
	return properties;
}
