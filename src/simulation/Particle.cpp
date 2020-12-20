#include <cstddef>
#include "Particle.h"

std::vector<StructProperty> const &Particle::GetProperties()
{
	static std::vector<StructProperty> properties = {
		{ "type"   , StructProperty::ParticleType, offsetof(Particle, type   ) },
		{ "life"   , StructProperty::ParticleType, offsetof(Particle, life   ) },
		{ "ctype"  , StructProperty::ParticleType, offsetof(Particle, ctype  ) },
		{ "x"      , StructProperty::Float       , offsetof(Particle, x      ) },
		{ "y"      , StructProperty::Float       , offsetof(Particle, y      ) },
		{ "vx"     , StructProperty::Float       , offsetof(Particle, vx     ) },
		{ "vy"     , StructProperty::Float       , offsetof(Particle, vy     ) },
		{ "temp"   , StructProperty::Float       , offsetof(Particle, temp   ) },
		{ "flags"  , StructProperty::UInteger    , offsetof(Particle, flags  ) },
		{ "tmp"    , StructProperty::Integer     , offsetof(Particle, tmp    ) },
		{ "tmp2"   , StructProperty::Integer     , offsetof(Particle, tmp2   ) },
		{ "dcolour", StructProperty::UInteger    , offsetof(Particle, dcolour) },
		{ "pavg0"  , StructProperty::Float       , offsetof(Particle, pavg[0]) },
		{ "pavg1"  , StructProperty::Float       , offsetof(Particle, pavg[1]) },
	};
	return properties;
}
