#include "Snapshot.h"

uint32_t Snapshot::Hash() const
{
	// http://www.isthe.com/chongo/tech/comp/fnv/
	auto hash = UINT32_C(2166136261);
	auto take = [&hash](const uint8_t *data, size_t size) {
		for (auto i = 0U; i < size; ++i)
		{
			hash ^= data[i];
			hash *= UINT32_C(16777619);
		}
	};
	auto takeThing = [&take](auto &thing) {
		take(reinterpret_cast<const uint8_t *>(&thing), sizeof(thing));
	};
	auto takeVector = [&take](auto &vec) {
		take(reinterpret_cast<const uint8_t *>(vec.data()), vec.size() * sizeof(vec[0]));
	};
	takeVector(AirPressure);
	takeVector(AirVelocityX);
	takeVector(AirVelocityY);
	takeVector(AmbientHeat);
	takeVector(Particles);
	takeVector(GravMass);
	takeVector(GravForceX);
	takeVector(GravForceY);
	takeVector(BlockMap);
	takeVector(ElecMap);
	takeVector(BlockAir);
	takeVector(BlockAirH);
	takeVector(FanVelocityX);
	takeVector(FanVelocityY);
	takeVector(PortalParticles);
	takeVector(WirelessData);
	takeVector(stickmen);
	takeThing(FrameCount);
	takeThing(RngState[0]);
	takeThing(RngState[1]);
	// signs and Authors are excluded on purpose, as they aren't POD and don't have much effect on the simulation.
	return hash;
}
