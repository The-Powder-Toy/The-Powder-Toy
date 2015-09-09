#include <cstddef>
#include "Particle.h"

std::vector<StructProperty> Particle::GetProperties()
{
	std::vector<StructProperty> properties;
	properties.push_back(StructProperty("type", StructProperty::ParticleType, offsetof(Particle, type)));
	properties.push_back(StructProperty("life", StructProperty::ParticleType, offsetof(Particle, life)));
	properties.push_back(StructProperty("ctype", StructProperty::ParticleType, offsetof(Particle, ctype)));
	properties.push_back(StructProperty("x", StructProperty::Float, offsetof(Particle, x)));
	properties.push_back(StructProperty("y", StructProperty::Float, offsetof(Particle, y)));
	properties.push_back(StructProperty("vx", StructProperty::Float, offsetof(Particle, vx)));
	properties.push_back(StructProperty("vy", StructProperty::Float, offsetof(Particle, vy)));
	properties.push_back(StructProperty("temp", StructProperty::Float, offsetof(Particle, temp)));
	properties.push_back(StructProperty("flags", StructProperty::UInteger, offsetof(Particle, flags)));
	properties.push_back(StructProperty("tmp", StructProperty::Integer, offsetof(Particle, tmp)));
	properties.push_back(StructProperty("tmp2", StructProperty::Integer, offsetof(Particle, tmp2)));
	properties.push_back(StructProperty("dcolour", StructProperty::UInteger, offsetof(Particle, dcolour)));
	properties.push_back(StructProperty("pavg0", StructProperty::Float, offsetof(Particle, pavg[0])));
	properties.push_back(StructProperty("pavg1", StructProperty::Float, offsetof(Particle, pavg[1])));
	return properties;
}
