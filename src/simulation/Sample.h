#pragma once
#include "Particle.h"

struct SimulationSample
{
	Particle particle;
	int ParticleID = 0;
	int PositionX = 0;
	int PositionY = 0;
	float AirPressure = 0;
	float AirTemperature = 0;
	float AirVelocityX = 0;
	float AirVelocityY = 0;

	int WallType = 0;
	float GravityVelocityX = 0;
	float GravityVelocityY = 0;

	int NumParts = 0;
	bool isMouseInSim = true;
};
