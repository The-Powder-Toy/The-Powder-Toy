//
//  GameSave.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//

#ifndef The_Powder_Toy_GameSave_h
#define The_Powder_Toy_GameSave_h

#include <vector>
#include "Misc.h"
#include "simulation/StorageClasses.h"

class GameSave
{
public:
	enum ParseResult { OK = 0, Corrupt, WrongVersion, InvalidDimensions, InternalError, MissingElement };
	
	int width, height;

	//Simulation data
	//int ** particleMap;
	int particlesCount;
	Particle * particles;
	char ** blockMap;
	float ** fanVelX;
	float ** fanVelY;
	
	//Simulation Options
	bool waterEEnabled;
	bool legacyEnable;
	bool gravityEnable;
	bool paused;
	int gravityMode;
	int airMode;
	
	//Signs
	std::vector<sign> signs;
	
	GameSave(GameSave & save);
	GameSave(int width, int height);
	GameSave(char * data, int dataSize);
	~GameSave();
	void setSize(int width, int height);
	char * Serialise(int & dataSize);
	void Transform(matrix2d transform, vector2d translate);
	
	inline GameSave& operator << (Particle v)
	{
		if(particlesCount<NPART && v.type)
		{
			particles[particlesCount++] = v;
		}
	}
	
	inline GameSave& operator << (sign v)
	{
		if(signs.size()<MAXSIGNS && v.text.length())
			signs.push_back(v);
	}
		
private:
	float * fanVelXPtr;
	float * fanVelYPtr;
	char * blockMapPtr;

	ParseResult readOPS(char * data, int dataLength);
	ParseResult readPSv(char * data, int dataLength);
	char * serialiseOPS(int & dataSize);
	//serialisePSv();
};

#endif
