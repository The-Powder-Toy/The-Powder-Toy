#ifndef The_Powder_Toy_GameSave_h
#define The_Powder_Toy_GameSave_h

#include <vector>
#include <string>
#include "Config.h"
#include "Misc.h"

#include "simulation/Sign.h"
#include "simulation/Particle.h"

//using namespace std;

struct ParseException: public std::exception {
	enum ParseResult { OK = 0, Corrupt, WrongVersion, InvalidDimensions, InternalError, MissingElement };
	std::string message;
	ParseResult result;
public:
	ParseException(ParseResult result, std::string message_): message(message_), result(result) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~ParseException() throw() {};
};

class GameSave
{
public:
	
	int blockWidth, blockHeight;

	//Simulation data
	//int ** particleMap;
	int particlesCount;
	Particle * particles;
	unsigned char ** blockMap;
	float ** fanVelX;
	float ** fanVelY;
	
	//Simulation Options
	bool waterEEnabled;
	bool legacyEnable;
	bool gravityEnable;
	bool aheatEnable;
	bool paused;
	int gravityMode;
	int airMode;
	
	//Signs
	std::vector<sign> signs;

	//Element palette
	typedef std::pair<std::string, int> PaletteItem;
	std::vector<PaletteItem> palette;
	
	GameSave();
	GameSave(GameSave & save);
	GameSave(int width, int height);
	GameSave(char * data, int dataSize);
	GameSave(std::vector<char> data);
	GameSave(std::vector<unsigned char> data);
	~GameSave();
	void setSize(int width, int height);
	char * Serialise(int & dataSize);
	std::vector<char> Serialise();
	void Transform(matrix2d transform, vector2d translate);

	void Expand();
	void Collapse();
	bool Collapsed();
	
	inline GameSave& operator << (Particle v)
	{
		if(particlesCount<NPART && v.type)
		{
			particles[particlesCount++] = v;
		}
		return *this;
	}
	
	inline GameSave& operator << (sign v)
	{
		if(signs.size()<MAXSIGNS && v.text.length())
			signs.push_back(v);
		return *this;
	}
		
private:
	bool expanded;
	bool hasOriginalData;
	float * fanVelXPtr;
	float * fanVelYPtr;
	unsigned char * blockMapPtr;

	std::vector<char> originalData;

	void dealloc();
	void read(char * data, int dataSize);
	void readOPS(char * data, int dataLength);
	void readPSv(char * data, int dataLength);
	char * serialiseOPS(int & dataSize);
	//serialisePSv();
};

#endif
