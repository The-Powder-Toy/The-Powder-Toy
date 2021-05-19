#include <cstddef>
#include "Particle.h"

std::vector<StructProperty> const &Particle::GetProperties()
{
	static std::vector<StructProperty> properties = {
		{ "type"   , StructProperty::ParticleType, (intptr_t)(offsetof(Particle, type   )) },
		{ "life"   , StructProperty::Integer     , (intptr_t)(offsetof(Particle, life   )) },
		{ "ctype"  , StructProperty::ParticleType, (intptr_t)(offsetof(Particle, ctype  )) },
		{ "x"      , StructProperty::Float       , (intptr_t)(offsetof(Particle, x      )) },
		{ "y"      , StructProperty::Float       , (intptr_t)(offsetof(Particle, y      )) },
		{ "vx"     , StructProperty::Float       , (intptr_t)(offsetof(Particle, vx     )) },
		{ "vy"     , StructProperty::Float       , (intptr_t)(offsetof(Particle, vy     )) },
		{ "temp"   , StructProperty::Float       , (intptr_t)(offsetof(Particle, temp   )) },
		{ "flags"  , StructProperty::UInteger    , (intptr_t)(offsetof(Particle, flags  )) },
		{ "tmp"    , StructProperty::Integer     , (intptr_t)(offsetof(Particle, tmp    )) },
		{ "tmp2"   , StructProperty::Integer     , (intptr_t)(offsetof(Particle, tmp2   )) },
		{ "dcolour", StructProperty::UInteger    , (intptr_t)(offsetof(Particle, dcolour)) },
		{ "pavg0"  , StructProperty::Float       , (intptr_t)(offsetof(Particle, pavg[0])) },
		{ "pavg1"  , StructProperty::Float       , (intptr_t)(offsetof(Particle, pavg[1])) },
	};
	return properties;
}
