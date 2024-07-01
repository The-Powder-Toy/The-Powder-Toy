#pragma once
#include "Particle.h"
#include "Sign.h"
#include "Stickman.h"
#include "common/tpt-rand.h"
#include <vector>
#include <array>
#include <json/json.h>

class Snapshot
{
public:
	std::vector<float> AirPressure;
	std::vector<float> AirVelocityX;
	std::vector<float> AirVelocityY;
	std::vector<float> AmbientHeat;

	std::vector<Particle> Particles;

	std::vector<float> GravForceX;
	std::vector<float> GravForceY;
	std::vector<float> GravMass;

	std::vector<unsigned char> BlockMap;
	std::vector<unsigned char> ElecMap;
	std::vector<unsigned char> BlockAir;
	std::vector<unsigned char> BlockAirH;

	std::vector<float> FanVelocityX;
	std::vector<float> FanVelocityY;


	std::vector<Particle> PortalParticles;
	std::vector<int> WirelessData;
	std::vector<playerst> stickmen;
	std::vector<sign> signs;

	uint64_t FrameCount;
	RNG::State RngState;

	uint32_t Hash() const;

	Json::Value Authors;

	virtual ~Snapshot() = default;
};
