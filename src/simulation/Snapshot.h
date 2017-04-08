#pragma once

#include <vector>

#include "Particle.h"

class Snapshot
{
public:
	std::vector<float> AirPressure;
	std::vector<float> AirVelocityX;
	std::vector<float> AirVelocityY;
	std::vector<float> AmbientHeat;

	std::vector<Particle> Particles;

	std::vector<float> GravVelocityX;
	std::vector<float> GravVelocityY;
	std::vector<float> GravValue;
	std::vector<float> GravMap;

	std::vector<unsigned char> BlockMap;
	std::vector<unsigned char> ElecMap;

	std::vector<float> FanVelocityX;
	std::vector<float> FanVelocityY;


	std::vector<Particle> PortalParticles;
	std::vector<int> WirelessData;
	std::vector<int> Wireless2Data;
	std::vector<playerst> stickmen;
	std::vector<sign> signs;

	Snapshot() :
		AirPressure(),
		AirVelocityX(),
		AirVelocityY(),
		AmbientHeat(),
		Particles(),
		GravVelocityX(),
		GravVelocityY(),
		GravValue(),
		GravMap(),
		BlockMap(),
		ElecMap(),
		FanVelocityX(),
		FanVelocityY(),
		PortalParticles(),
		WirelessData(),
		Wireless2Data(),
		stickmen(),
		signs()
	{

	}

	virtual ~Snapshot()
	{

	}
};
