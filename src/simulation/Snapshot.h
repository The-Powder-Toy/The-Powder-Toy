#pragma once

#include <vector>

#include "Particle.h"

class Snapshot
{
public:
	std::vector<float> AirPressure;
	std::vector<float> AirVelocityX;
	std::vector<float> AirVelocityY;

	std::vector<Particle> Particles;
	std::vector<Particle> PortalParticles;

	std::vector<int> WirelessData;

	std::vector<float> GravVelocityX;
	std::vector<float> GravVelocityY;
	std::vector<float> GravValue;
	std::vector<float> GravMap;

	std::vector<unsigned char> BlockMap;
	std::vector<unsigned char> ElecMap;

	std::vector<float> FanVelocityX;
	std::vector<float> FanVelocityY;

	Snapshot() :
		AirPressure(),
		AirVelocityX(),
		AirVelocityY(),
		Particles(),
		PortalParticles(),
		WirelessData(),
		GravVelocityX(),
		GravVelocityY(),
		GravValue(),
		GravMap(),
		BlockMap(),
		ElecMap(),
		FanVelocityX(),
		FanVelocityY()
	{

	}

	virtual ~Snapshot()
	{

	}
};