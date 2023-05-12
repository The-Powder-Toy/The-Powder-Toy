#pragma once
#include "common/Plane.h"
#include "common/String.h"
#include "common/tpt-rand.h"
#include "simulation/Sign.h"
#include "simulation/Particle.h"
#include "Misc.h"
#include "SimulationConfig.h"
#include <vector>
#include <array>
#include <json/json.h>

struct sign;
struct Particle;

struct ParseException: public std::exception {
	enum ParseResult { OK = 0, Corrupt, WrongVersion, InvalidDimensions, InternalError, MissingElement };
	ByteString message;
	ParseResult result;
public:
	ParseException(ParseResult result, String message): message(message.ToUtf8()), result(result) {}
	const char * what() const throw() override
	{
		return message.c_str();
	}
	~ParseException() throw() {}
};

struct BuildException: public std::exception {
	ByteString message;
public:
	BuildException(String message): message(message.ToUtf8()) {}
	const char * what() const throw() override
	{
		return message.c_str();
	}
	~BuildException() throw() {}
};

class StkmData
{
public:
	bool rocketBoots1 = false;
	bool rocketBoots2 = false;
	bool fan1 = false;
	bool fan2 = false;
	std::vector<unsigned int> rocketBootsFigh;
	std::vector<unsigned int> fanFigh;

	bool hasData() const
	{
		return rocketBoots1 || rocketBoots2 || fan1 || fan2
		        || rocketBootsFigh.size() || fanFigh.size();
	}
};

class GameSave
{
	// number of pixels translated. When translating CELL pixels, shift all CELL grids
	vector2d translated = { 0, 0 };
	void readOPS(const std::vector<char> &data);
	void readPSv(const std::vector<char> &data);
	std::pair<bool, std::vector<char>> serialiseOPS() const;

public:
	Vec2<int> blockSize = { 0, 0 };
	bool fromNewerVersion = false;
	int majorVersion = 0;
	int minorVersion = 0;
	bool hasPressure = false;
	bool hasAmbientHeat = false;
	bool hasBlockAirMaps = false; // only written by readOPS, never read
	bool ensureDeterminism = false; // only taken seriously by serializeOPS; readOPS may set this even if the save does not have everything required for determinism
	bool hasRngState = false; // only written by readOPS, never read
	RNG::State rngState;
	uint64_t frameCount = 0;

	//Simulation data
	int particlesCount = 0;
	std::vector<Particle> particles;
	PlaneAdapter<std::vector<unsigned char>> blockMap;
	PlaneAdapter<std::vector<float>> fanVelX;
	PlaneAdapter<std::vector<float>> fanVelY;
	PlaneAdapter<std::vector<float>> pressure;
	PlaneAdapter<std::vector<float>> velocityX;
	PlaneAdapter<std::vector<float>> velocityY;
	PlaneAdapter<std::vector<float>> ambientHeat;
	PlaneAdapter<std::vector<unsigned char>> blockAir;
	PlaneAdapter<std::vector<unsigned char>> blockAirh;

	//Simulation Options
	bool waterEEnabled = false;
	bool legacyEnable = false;
	bool gravityEnable = false;
	bool aheatEnable = false;
	bool paused = false;
	int gravityMode = 0;
	float customGravityX = 0.0f;
	float customGravityY = 0.0f;
	int airMode = 0;
	float ambientAirTemp = R_TEMP + 273.15f;
	int edgeMode = 0;
	bool wantAuthors = true;

	//Signs
	std::vector<sign> signs;
	StkmData stkm;

	//Element palette
	typedef std::pair<ByteString, int> PaletteItem;
	std::vector<PaletteItem> palette;

	// author information
	Json::Value authors;

	int pmapbits = 8; // default to 8 bits for older saves

	GameSave(Vec2<int> newBlockSize);
	GameSave(const std::vector<char> &data, bool newWantAuthors = true);
	void setSize(Vec2<int> newBlockSize);
	// return value is [ fakeFromNewerVersion, gameData ]
	std::pair<bool, std::vector<char>> Serialise() const;
	vector2d Translate(vector2d translate);
	void Transform(matrix2d transform, vector2d translate);
	void Transform(matrix2d transform, vector2d translate, vector2d translateReal, Vec2<int> newPartSize);

	void Expand(const std::vector<char> &data);

	static bool PressureInTmp3(int type);

	GameSave& operator << (Particle &v);
	GameSave& operator << (sign &v);
};
