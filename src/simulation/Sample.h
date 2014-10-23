

#ifndef The_Powder_Toy_Sample_h
#define The_Powder_Toy_Sample_h

#include "Particle.h"

class SimulationSample
{
public:
	Particle particle;
	int ParticleID;
	int PositionX, PositionY;
	float AirPressure;
	float AirTemperature;
	float AirVelocityX;
	float AirVelocityY;

	int WallType;
	float Gravity;
	float GravityVelocityX;
	float GravityVelocityY;

	int NumParts;
	bool isMouseInSim;

	SimulationSample() : particle(), ParticleID(0), PositionX(0), PositionY(0), AirPressure(0), AirTemperature(0), AirVelocityX(0), AirVelocityY(0), WallType(0), Gravity(0), GravityVelocityX(0), GravityVelocityY(0), NumParts(0), isMouseInSim(true) {}
};

#endif
