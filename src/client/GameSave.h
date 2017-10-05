#ifndef The_Powder_Toy_GameSave_h
#define The_Powder_Toy_GameSave_h

#include <vector>
#include <string>
#include "Config.h"
#include "Misc.h"

#include "bson/BSON.h"
#include "json/json.h"
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
	~ParseException() throw() {}
};

class GameSave
{
public:
	
	int blockWidth, blockHeight;
	bool fromNewerVersion;
	bool hasAmbientHeat;

	//Simulation data
	//int ** particleMap;
	int particlesCount;
	Particle * particles;
	unsigned char ** blockMap;
	float ** fanVelX;
	float ** fanVelY;
	float ** pressure;
	float ** velocityX;
	float ** velocityY;
	float ** ambientHeat;
	
	//Simulation Options
	bool waterEEnabled;
	bool legacyEnable;
	bool gravityEnable;
	bool aheatEnable;
	bool paused;
	int gravityMode;
	int airMode;
	int edgeMode;
	
	//Signs
	std::vector<sign> signs;

	//Element palette
	typedef std::pair<std::string, int> PaletteItem;
	std::vector<PaletteItem> palette;

	// author information
	Json::Value authors;

	GameSave();
	GameSave(GameSave & save);
	GameSave(int width, int height);
	GameSave(char * data, int dataSize);
	GameSave(std::vector<char> data);
	GameSave(std::vector<unsigned char> data);
	~GameSave();
	void setSize(int width, int height);
	char * Serialise(unsigned int & dataSize);
	std::vector<char> Serialise();
	vector2d Translate(vector2d translate);
	void Transform(matrix2d transform, vector2d translate);
	void Transform(matrix2d transform, vector2d translate, vector2d translateReal, int newWidth, int newHeight);

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
	// number of pixels translated. When translating CELL pixels, shift all CELL grids
	vector2d translated;

	std::vector<char> originalData;

	void InitData();
	void InitVars();
	void CheckBsonFieldUser(bson_iterator iter, const char *field, unsigned char **data, unsigned int *fieldLen);
	void CheckBsonFieldBool(bson_iterator iter, const char *field, bool *flag);
	void CheckBsonFieldInt(bson_iterator iter, const char *field, int *setting);
	template <typename T> T ** Allocate2DArray(int blockWidth, int blockHeight, T defaultVal);
	template <typename T> void Deallocate2DArray(T ***array, int blockHeight);
	void dealloc();
	void read(char * data, int dataSize);
	void readOPS(char * data, int dataLength);
	void readPSv(char * data, int dataLength);
	char * serialiseOPS(unsigned int & dataSize);
	void ConvertJsonToBson(bson *b, Json::Value j, int depth = 0);
	void ConvertBsonToJson(bson_iterator *b, Json::Value *j, int depth = 0);
};

#endif
