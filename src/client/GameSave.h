#pragma once
#include "common/String.h"
#include "simulation/Sign.h"
#include "simulation/Particle.h"
#include "SimulationConfig.h"
#include <vector>
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

template<class Item>
struct Plane
{
	int width = 0;
	int height = 0;
	std::vector<Item> items;
	// invariant: items.size() == width * height

	Item *operator [](int y)
	{
		return &items[y * width];
	}

	const Item *operator [](int y) const
	{
		return &items[y * width];
	}

	Plane() = default;
	Plane(int newWidth, int newHeight, Item defaultVal) : width(newWidth), height(newHeight), items(width * height, defaultVal)
	{
	}
};

class GameSave
{
	// number of pixels translated. When translating CELL pixels, shift all CELL grids
	Vec2<float> translated = Vec2<float>::Zero;
	void readOPS(const std::vector<char> &data);
	void readPSv(const std::vector<char> &data);
	std::pair<bool, std::vector<char>> serialiseOPS() const;

public:
	Vec2<int> blockDimen = Vec2<int>::Zero;
	bool fromNewerVersion = false;
	int majorVersion = 0;
	int minorVersion = 0;
	bool hasPressure = false;
	bool hasAmbientHeat = false;

	//Simulation data
	int particlesCount = 0;
	std::vector<Particle> particles;
	Plane<unsigned char> blockMap;
	Plane<float> fanVelX;
	Plane<float> fanVelY;
	Plane<float> pressure;
	Plane<float> velocityX;
	Plane<float> velocityY;
	Plane<float> ambientHeat;

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

	GameSave(Vec2<int> blockDimen);
	GameSave(const std::vector<char> &data, bool newWantAuthors = true);
	void setSize(Vec2<int> dimen);
	// return value is [ fakeFromNewerVersion, gameData ]
	std::pair<bool, std::vector<char>> Serialise() const;
	Vec2<float> Translate(Vec2<float> translate);
	void Transform(Mat2<float> transform, Vec2<float> translate);
	void Transform(Mat2<float> transform, Vec2<float> translate, Vec2<float> translateReal, Vec2<int> newDimen);

	void Expand(const std::vector<char> &data);

	static bool PressureInTmp3(int type);

	GameSave& operator << (Particle &v);
	GameSave& operator << (sign &v);
};
