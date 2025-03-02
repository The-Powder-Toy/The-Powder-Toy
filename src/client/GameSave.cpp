#include "GameSave.h"
#include "bzip2/bz2wrap.h"
#include "Format.h"
#include "simulation/Simulation.h"
#include "simulation/ElementClasses.h"
#include "simulation/elements/PIPE.h"
#include "common/tpt-compat.h"
#include "bson/BSON.h"
#include "graphics/Renderer.h"
#include "Config.h"
#include <iostream>
#include <cmath>
#include <climits>
#include <memory>
#include <set>
#include <cmath>
#include <algorithm>

constexpr auto currentVersion = UPSTREAM_VERSION.displayVersion;
constexpr auto nextVersion = Version(99, 3);
static_assert(!ALLOW_FAKE_NEWER_VERSION || nextVersion >= currentVersion);

constexpr auto effectiveVersion = ALLOW_FAKE_NEWER_VERSION ? nextVersion : currentVersion;

static void ConvertJsonToBson(bson *b, Json::Value j, int depth = 0);
static void ConvertBsonToJson(bson_iterator *b, Json::Value *j, int depth = 0);
static void CheckBsonFieldUser(bson_iterator iter, const char *field, unsigned char **data, unsigned int *fieldLen);
static void CheckBsonFieldBool(bson_iterator iter, const char *field, bool *flag);
static void CheckBsonFieldInt(bson_iterator iter, const char *field, int *setting);
static void CheckBsonFieldLong(bson_iterator iter, const char *field, int64_t *setting);
static void CheckBsonFieldFloat(bson_iterator iter, const char *field, float *setting);

GameSave::GameSave(Vec2<int> newBlockSize)
{
	setSize(newBlockSize);
}

GameSave::GameSave(const std::vector<char> &data, bool newWantAuthors)
{
	wantAuthors = newWantAuthors;

	try
	{
		Expand(data);
	}
	catch(ParseException & e)
	{
		std::cout << e.what() << std::endl;
		throw;
	}
}

void GameSave::MapPalette()
{
	// - the palette is always right
	//   - there are palettes with missing entries but there are no palettes with incorrect entries
	//   - for every (identifier, number) pair in the palette
	//     - if the identifier is recognized, map the number to the appropriate element
	//     - if not, map it to 0
	//       - complain about the identifier if the corresponding number is actually used
	// - to handle every number not covered by the palette
	//   - in the case of any 98.0+ save, the palette is comprehensive
	//     - except for the few cases already handled below
	//       - e.g RSSS et al not having CarriesTypeIn set properly until 98.2
	//       - can handle mistakes like this with similar extra code later
	//     - map any number seen used to 0 and complain about the number
	//   - in the case of any pre-98.0 save, the palette may not be comprehensive
	//     - in the case of saves from 78.1 and newer
	//       - identity-map only ranges of numbers that are known to have existed
	//         at the point in time in vanilla when the save was made, based on this->version
	//       - this is still not perfect because it lets numbers slip that a mod freed up a
	//         vanilla element from and reused for one of its own elements, but that's fine
	//     - in the case of saves older than that
	//       - pretend that they are 78.1 in terms of validity of element numbers, this is good enough

	std::vector<int> partMap(PT_NUM, 0);
	std::vector<bool> ignoreMissingErrors(PT_NUM, false);
	if (version <= Version(98, 2))
	{
		ignoreMissingErrors[PT_ICEI] = true;
		ignoreMissingErrors[PT_SNOW] = true;
		ignoreMissingErrors[PT_RSST] = true;
		ignoreMissingErrors[PT_RSSS] = true;
	}

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	std::map<ByteString, int> missingElementIdentifiers;
	if (version < Version(98, 0))
	{
		struct CoarsePaletteInfo
		{
			Version<2> firstVersion; // the first version to which this entry applies
			int maxValid;            // almost all element numbers in the range [1, maxValid] are valid, but see golHoleFirst
			int golHoleFirst;        // the element numbers in the range [golHoleFirst, golHoleLast] are not valid
			int golHoleLast;
		};
		static const std::vector<CoarsePaletteInfo> cpi = {
			// must be sorted by firstVersion
			{ Version(78, 1), 160, 144, 146 }, // 147 isn't allocated in this version but other elements
			                                   // in certain states are loaded as 147, so leave it alone
			{ Version(79, 0), 160, 145, 146 }, // similarly
			{ Version(80, 0), 160, 146, 146 }, // similarly
			{ Version(80, 5), 160, 146, 146 },
			{ Version(81, 7), 161, 146, 146 },
			{ Version(83, 0), 162, 146, 146 },
			{ Version(83, 4), 163, 146, 146 },
			{ Version(84, 0), 166, 146, 146 },
			{ Version(86, 0), 169, 146, 146 },
			{ Version(87, 1), 172, 146, 146 },
			{ Version(89, 0), 176, 146, 146 },
			{ Version(90, 0), 178, 146, 146 },
			{ Version(91, 0), 179, 146, 146 },
			{ Version(92, 0), 185, 146, 146 },
			{ Version(94, 0), 186, 146, 146 },
			{ Version(96, 0), 191, 146, 146 },
		};
		auto found = cpi[0]; // pretend everything is at least from the first version in the list
		for (auto &info : std::span(cpi.begin() + 1, cpi.end()))
		{
			if (info.firstVersion <= version)
			{
				found = info;
			}
		}
		for (int i = 1; i <= found.maxValid; i++)
		{
			if (i >= found.golHoleFirst && i <= found.golHoleLast)
			{
				continue;
			}
			partMap[i] = i;
		}
	}
	for (auto &pi : palette)
	{
		if (pi.second > 0 && pi.second < PT_NUM)
		{
			int myId = 0;
			for (int i = 0; i < PT_NUM; i++)
			{
				if (elements[i].Enabled && elements[i].Identifier == pi.first)
				{
					myId = i;
				}
			}
			partMap[pi.second] = myId;
			if (!myId)
			{
				missingElementIdentifiers.insert(pi);
			}
		}
	}
	auto paletteLookup = [this, &partMap](int type, bool ignoreMissingErrors) {
		if (type > 0 && type < PT_NUM)
		{
			auto carriedType = partMap[type];
			if (!carriedType) // type is not 0 so this shouldn't be 0 either
			{
				if (ignoreMissingErrors)
					return type;
				missingElements.ids.insert(type);
			}
			type = carriedType;
		}
		return type;
	};

	unsigned int pmapmask = (1<<pmapbits)-1;
	auto &possiblyCarriesType = Particle::PossiblyCarriesType();
	auto &properties = Particle::GetProperties();
	for (int n = 0; n < NPART && n < particlesCount; n++)
	{
		Particle &tempPart = particles[n];
		if (tempPart.type <= 0 || tempPart.type >= PT_NUM)
		{
			continue;
		}
		tempPart.type = paletteLookup(tempPart.type, false);
		for (auto index : possiblyCarriesType)
		{
			if (elements[tempPart.type].CarriesTypeIn & (1U << index))
			{
				auto *prop = reinterpret_cast<int *>(reinterpret_cast<char *>(&tempPart) + properties[index].Offset);
				auto carriedType = *prop & int(pmapmask);
				auto extra = *prop >> pmapbits;
				carriedType = paletteLookup(carriedType, ignoreMissingErrors[tempPart.type]);
				*prop = PMAP(extra, carriedType);
			}
		}
	}
	for (const auto &pi : missingElementIdentifiers)
	{
		if (missingElements.ids.find(pi.second) != missingElements.ids.end())
		{
			missingElements.identifiers.insert(pi);
		}
	}
}

void GameSave::Expand(const std::vector<char> &data)
{
	try
	{
	if(data.size() > 15)
	{
		if ((data[0]==0x66 && data[1]==0x75 && data[2]==0x43) || (data[0]==0x50 && data[1]==0x53 && data[2]==0x76))
		{
			readPSv(data);
		}
		else if(data[0] == 'O' && data[1] == 'P' && data[2] == 'S')
		{
			if (data[3] != '1')
				throw ParseException(ParseException::WrongVersion, "Save format from newer version");
			readOPS(data);
		}
		else
		{
			std::cerr << "Got Magic number '" << data[0] << data[1] << data[2] << "'" << std::endl;
			throw ParseException(ParseException::Corrupt, "Invalid save format");
		}
		MapPalette();
	}
	else
	{
		throw ParseException(ParseException::Corrupt, "No data");
	}
	}
	catch (const std::bad_alloc &)
	{
		throw ParseException(ParseException::Corrupt, "Cannot allocate memory");
	}
}

void GameSave::setSize(Vec2<int> newBlockSize)
{
	blockSize = newBlockSize;

	particlesCount = 0;
	particles = std::vector<Particle>(NPART);

	blockMap = PlaneAdapter<std::vector<unsigned char>>(blockSize, 0);
	fanVelX = PlaneAdapter<std::vector<float>>(blockSize, 0.0f);
	fanVelY = PlaneAdapter<std::vector<float>>(blockSize, 0.0f);
	pressure = PlaneAdapter<std::vector<float>>(blockSize, 0.0f);
	velocityX = PlaneAdapter<std::vector<float>>(blockSize, 0.0f);
	velocityY = PlaneAdapter<std::vector<float>>(blockSize, 0.0f);
	ambientHeat = PlaneAdapter<std::vector<float>>(blockSize, 0.0f);
	blockAir = PlaneAdapter<std::vector<unsigned char>>(blockSize, 0);
	blockAirh = PlaneAdapter<std::vector<unsigned char>>(blockSize, 0);
	gravMass = PlaneAdapter<std::vector<float>>(blockSize, 0.f);
	gravMask = PlaneAdapter<std::vector<uint32_t>>(blockSize, UINT32_C(0xFFFFFFFF));
	gravForceX = PlaneAdapter<std::vector<float>>(blockSize, 0.f);
	gravForceY = PlaneAdapter<std::vector<float>>(blockSize, 0.f);
}

std::pair<bool, std::vector<char>> GameSave::Serialise() const
{
	try
	{
		return serialiseOPS();
	}
	catch (const std::bad_alloc &)
	{
		std::cout << "Save error, out of memory" << std::endl;
	}
	catch (BuildException & e)
	{
		std::cout << e.what() << std::endl;
	}
	return { false, {} };
}

void GameSave::Transform(Mat2<int> transform, Vec2<int> nudge)
{
	// undo translation by rotation
	auto br  = transform * (blockSize * CELL - Vec2{ 1, 1 });
	auto bbr = transform * (blockSize        - Vec2{ 1, 1 });
	auto translate  = Vec2{ std::max(0,  -br.X), std::max(0,  -br.Y) };
	auto btranslate = Vec2{ std::max(0, -bbr.X), std::max(0, -bbr.Y) };
	auto newBlockS = transform * blockSize;
	newBlockS.X = std::abs(newBlockS.X);
	newBlockS.Y = std::abs(newBlockS.Y);
	translate += nudge;

	// Grow as needed.
	assert((Vec2{ CELL, CELL }.OriginRect().Contains(nudge)));
	if (nudge.X) newBlockS.X += 1;
	if (nudge.Y) newBlockS.Y += 1;

	// TODO: allow transforms to yield bigger saves. For this we'd need SaveRenderer (the singleton, not Renderer)
	// to fully render them (possible with stitching) and Simulation::Load to be able to take only the part that fits.
	newBlockS = newBlockS.Clamp(RectBetween({ 0, 0 }, CELLS));
	auto newPartS = newBlockS * CELL;

	// Prepare to patch pipes.
	std::array<int, 8> pipeOffsetMap;
	{
		std::transform(Element_PIPE_offsets.begin(), Element_PIPE_offsets.end(), pipeOffsetMap.begin(), [transform](auto offset) {
			auto it = std::find(Element_PIPE_offsets.begin(), Element_PIPE_offsets.end(), transform * offset);
			assert(it != Element_PIPE_offsets.end());
			return int(it - Element_PIPE_offsets.begin());
		});
	}

	// Translate signs.
	for (auto i = 0U; i < signs.size(); i++)
	{
		auto newPos = transform * Vec2{ signs[i].x, signs[i].y } + translate;
		if (!newPartS.OriginRect().Contains(newPos))
		{
			signs[i].text.clear();
			continue;
		}
		signs[i].x = newPos.X;
		signs[i].y = newPos.Y;
	}

	// Translate particles.
	for (int i = 0; i < particlesCount; i++)
	{
		if (!particles[i].type)
		{
			continue;
		}
		{
			// * We want particles to retain their distance from the centre of the particle grid cell
			//   they are in, but more importantly we also don't want them to change grid cells,
			//   so we just get rid of the edge cases.
			constexpr auto threshold = 0.001f;
			auto boundaryDiffX = particles[i].x - (floor(particles[i].x) + 0.5f);
			if (fabs(boundaryDiffX) < threshold)
			{
				particles[i].x += copysign(threshold, boundaryDiffX);
			}
			auto boundaryDiffY = particles[i].y - (floor(particles[i].y) + 0.5f);
			if (fabs(boundaryDiffY) < threshold)
			{
				particles[i].y += copysign(threshold, boundaryDiffY);
			}
		}
		if (particles[i].x - floor(particles[i].x) == 0.5f) particles[i].x += 0.001f;
		if (particles[i].y - floor(particles[i].y) == 0.5f) particles[i].y += 0.001f;
		auto newPos = transform * Vec2{ particles[i].x, particles[i].y } + translate;
		if (!newPartS.OriginRect().Contains(Vec2{ int(floor(newPos.X + 0.5f)), int(floor(newPos.Y + 0.5f)) }))
		{
			particles[i].type = PT_NONE;
			continue;
		}
		particles[i].x = newPos.X;
		particles[i].y = newPos.Y;
		auto newVel = transform * Vec2{ particles[i].vx, particles[i].vy };
		particles[i].vx = newVel.X;
		particles[i].vy = newVel.Y;
		if (particles[i].type == PT_PIPE || particles[i].type == PT_PPIP)
		{
			Element_PIPE_transformPatchOffsets(particles[i], pipeOffsetMap);
		}
	}

	// Translate blocky stuff.
	PlaneAdapter<std::vector<unsigned char>> newBlockMap(newBlockS, 0);
	PlaneAdapter<std::vector<float>> newFanVelX(newBlockS, 0.0f);
	PlaneAdapter<std::vector<float>> newFanVelY(newBlockS, 0.0f);
	PlaneAdapter<std::vector<float>> newPressure(newBlockS, 0.0f);
	PlaneAdapter<std::vector<float>> newVelocityX(newBlockS, 0.0f);
	PlaneAdapter<std::vector<float>> newVelocityY(newBlockS, 0.0f);
	PlaneAdapter<std::vector<float>> newAmbientHeat(newBlockS, 0.0f);
	PlaneAdapter<std::vector<unsigned char>> newBlockAir(newBlockS, 0);
	PlaneAdapter<std::vector<unsigned char>> newBlockAirh(newBlockS, 0);
	PlaneAdapter<std::vector<float>> newGravMass(newBlockS, 0.f);
	PlaneAdapter<std::vector<uint32_t>> newGravMask(newBlockS, UINT32_C(0xFFFFFFFF));
	PlaneAdapter<std::vector<float>> newGravForceX(newBlockS, 0.f);
	PlaneAdapter<std::vector<float>> newGravForceY(newBlockS, 0.f);
	for (auto bpos : blockSize.OriginRect())
	{
		auto newBpos = transform * bpos + btranslate;
		if (!newBlockS.OriginRect().Contains(newBpos))
		{
			continue;
		}
		if (blockMap[bpos])
		{
			newBlockMap[newBpos] = blockMap[bpos];
			if (blockMap[bpos] == WL_FAN)
			{
				auto newVel = transform * Vec2{ fanVelX[bpos], fanVelY[bpos] };
				newFanVelX[newBpos] = newVel.X;
				newFanVelY[newBpos] = newVel.Y;
			}
		}
		newPressure[newBpos] = pressure[bpos];
		newVelocityX[newBpos] = velocityX[bpos];
		newVelocityY[newBpos] = velocityY[bpos];
		newAmbientHeat[newBpos] = ambientHeat[bpos];
		newBlockAir[newBpos] = blockAir[bpos];
		newBlockAirh[newBpos] = blockAirh[bpos];
		newGravMass[newBpos] = gravMass[bpos];
		newGravMask[newBpos] = gravMask[bpos];
		newGravForceX[newBpos] = gravForceX[bpos];
		newGravForceY[newBpos] = gravForceY[bpos];
	}
	blockMap = std::move(newBlockMap);
	fanVelX = std::move(newFanVelX);
	fanVelY = std::move(newFanVelY);
	pressure = std::move(newPressure);
	velocityX = std::move(newVelocityX);
	velocityY = std::move(newVelocityY);
	ambientHeat = std::move(newAmbientHeat);
	blockAir = std::move(newBlockAir);
	blockAirh = std::move(newBlockAirh);
	gravMass = std::move(newGravMass);
	gravMask = std::move(newGravMask);
	gravForceX = std::move(newGravForceX);
	gravForceY = std::move(newGravForceY);

	blockSize = newBlockS;
}

static void CheckBsonFieldUser(bson_iterator iter, const char *field, unsigned char **data, unsigned int *fieldLen)
{
	if (!strcmp(bson_iterator_key(&iter), field))
	{
		if (bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (*fieldLen = bson_iterator_bin_len(&iter)) > 0)
		{
			*data = (unsigned char*)bson_iterator_bin_data(&iter);
		}
		else
		{
			fprintf(stderr, "Invalid datatype for %s: %d[%d] %d[%d] %d[%d]\n", field, bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
		}
	}
}

static void CheckBsonFieldBool(bson_iterator iter, const char *field, bool *flag)
{
	if (!strcmp(bson_iterator_key(&iter), field))
	{
		if (bson_iterator_type(&iter) == BSON_BOOL)
		{
			*flag = bson_iterator_bool(&iter);
		}
		else
		{
			fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
		}
	}
}

static void CheckBsonFieldInt(bson_iterator iter, const char *field, int *setting)
{
	if (!strcmp(bson_iterator_key(&iter), field))
	{
		if (bson_iterator_type(&iter) == BSON_INT)
		{
			*setting = bson_iterator_int(&iter);
		}
		else
		{
			fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
		}
	}
}

static void CheckBsonFieldLong(bson_iterator iter, const char *field, int64_t *setting)
{
	if (!strcmp(bson_iterator_key(&iter), field))
	{
		if (bson_iterator_type(&iter) == BSON_LONG)
		{
			*setting = bson_iterator_long(&iter);
		}
		else
		{
			fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
		}
	}
}

static void CheckBsonFieldFloat(bson_iterator iter, const char *field, float *setting)
{
	if (!strcmp(bson_iterator_key(&iter), field))
	{
		if (bson_iterator_type(&iter) == BSON_DOUBLE)
		{
			*setting = float(bson_iterator_double(&iter));
		}
		else
		{
			fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
		}
	}
}

void GameSave::readOPS(const std::vector<char> &data)
{
	auto &builtinGol = SimulationData::builtinGol;

	Renderer::PopulateTables();

	auto *inputData = reinterpret_cast<const unsigned char *>(data.data());
	unsigned char *partsData = nullptr;
	unsigned char *partsPosData = nullptr;
	unsigned char *fanData = nullptr;
	unsigned char *wallData = nullptr;
	unsigned char *soapLinkData = nullptr;
	unsigned char *pressData = nullptr, *vxData = nullptr, *vyData = nullptr, *ambientData = nullptr, *blockAirData = nullptr, *gravityData = nullptr;
	unsigned int inputDataLen = data.size(), bsonDataLen = 0, partsDataLen, partsPosDataLen, fanDataLen, wallDataLen, soapLinkDataLen;
	unsigned int pressDataLen, vxDataLen, vyDataLen, ambientDataLen, blockAirDataLen, gravityDataLen;
	unsigned partsCount = 0;
	unsigned int savedVersion = inputData[4];
	version = { savedVersion, 0 };
	bool fakeNewerVersion = false; // used for development builds only

	bson b;
	b.data = nullptr;
	auto bson_deleter = [](bson * b) { bson_destroy(b); };
	// Use unique_ptr with a custom deleter to ensure that bson_destroy is called even when an exception is thrown
	std::unique_ptr<bson, decltype(bson_deleter)> b_ptr(&b, bson_deleter);

	//Block sizes
	auto blockP = Vec2{ 0, 0 };
	auto blockS = Vec2{ int(inputData[6]), int(inputData[7]) };

	//Full size, normalised
	auto partP = blockP * CELL;
	auto partS = blockS * CELL;

	//Incompatible cell size
	if (inputData[5] != CELL)
		throw ParseException(ParseException::InvalidDimensions, "Incorrect CELL size");

	if (!RectBetween({ 0, 0 }, CELLS).Contains(blockS))
		throw ParseException(ParseException::InvalidDimensions, "Save is of invalid size");

	//Too large/off screen
	if (!RectBetween({ 0, 0 }, CELLS).Contains(blockP + blockS))
		throw ParseException(ParseException::InvalidDimensions, "Save extends beyond canvas");

	setSize(blockS);

	bsonDataLen = ((unsigned)inputData[8]);
	bsonDataLen |= ((unsigned)inputData[9]) << 8;
	bsonDataLen |= ((unsigned)inputData[10]) << 16;
	bsonDataLen |= ((unsigned)inputData[11]) << 24;

	//Check for overflows, don't load saves larger than 200MB
	unsigned int toAlloc = bsonDataLen;
	if (toAlloc > 209715200 || !toAlloc)
		throw ParseException(ParseException::InvalidDimensions, "Save data too large, refusing");

	{
		std::vector<char> bsonData;
		switch (auto status = BZ2WDecompress(bsonData, std::span(reinterpret_cast<const char *>(inputData + 12), inputDataLen - 12), toAlloc))
		{
		case BZ2WDecompressOk: break;
		case BZ2WDecompressNomem: throw ParseException(ParseException::Corrupt, "Cannot allocate memory");
		default: throw ParseException(ParseException::Corrupt, String::Build("Cannot decompress: status ", int(status)));
		}

		bsonDataLen = bsonData.size();
		//Make sure bsonData is null terminated, since all string functions need null terminated strings
		//(bson_iterator_key returns a pointer into bsonData, which is then used with strcmp)
		bsonData.push_back(0);

		// apparently bson_* takes ownership of the data passed into it?????????
		auto *pleaseFixMe = (char *)malloc(bsonData.size());
		std::copy(bsonData.begin(), bsonData.end(), pleaseFixMe);
		bson_init_data_size(&b, pleaseFixMe, bsonDataLen);
	}

	set_bson_err_handler([](const char* err) { throw ParseException(ParseException::Corrupt, "BSON error when parsing save: " + ByteString(err).FromUtf8()); });

	std::vector<sign> tempSigns;

	{
		// find origin first so version is accurate by the time checks against it are made
		bson_iterator iter;
		bson_iterator_init(&iter, &b);
		while (bson_iterator_next(&iter))
		{
			if (!strcmp(bson_iterator_key(&iter), "origin"))
			{
				if (bson_iterator_type(&iter) == BSON_OBJECT)
				{
					bson_iterator subiter;
					bson_iterator_subiterator(&iter, &subiter);
					while (bson_iterator_next(&subiter))
					{
						if (bson_iterator_type(&subiter) == BSON_INT)
						{
							if (!strcmp(bson_iterator_key(&subiter), "minorVersion"))
							{
								version[1] = bson_iterator_int(&subiter);
							}
						}
					}
				}
				else
				{
					fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
				}
			}
		}
	}
	fromNewerVersion = version > currentVersion;

	bson_iterator iter;
	bson_iterator_init(&iter, &b);
	while (bson_iterator_next(&iter))
	{
		CheckBsonFieldUser(iter, "parts", &partsData, &partsDataLen);
		CheckBsonFieldUser(iter, "partsPos", &partsPosData, &partsPosDataLen);
		CheckBsonFieldUser(iter, "wallMap", &wallData, &wallDataLen);
		CheckBsonFieldUser(iter, "pressMap", &pressData, &pressDataLen);
		CheckBsonFieldUser(iter, "vxMap", &vxData, &vxDataLen);
		CheckBsonFieldUser(iter, "vyMap", &vyData, &vyDataLen);
		CheckBsonFieldUser(iter, "ambientMap", &ambientData, &ambientDataLen);
		CheckBsonFieldUser(iter, "blockAir", &blockAirData, &blockAirDataLen);
		CheckBsonFieldUser(iter, "gravity", &gravityData, &gravityDataLen);
		CheckBsonFieldUser(iter, "fanMap", &fanData, &fanDataLen);
		CheckBsonFieldUser(iter, "soapLinks", &soapLinkData, &soapLinkDataLen);
		CheckBsonFieldBool(iter, "legacyEnable", &legacyEnable);
		CheckBsonFieldBool(iter, "gravityEnable", &gravityEnable);
		CheckBsonFieldBool(iter, "aheat_enable", &aheatEnable);
		CheckBsonFieldBool(iter, "waterEEnabled", &waterEEnabled);
		CheckBsonFieldBool(iter, "paused", &paused);
		CheckBsonFieldInt(iter, "gravityMode", &gravityMode);
		CheckBsonFieldFloat(iter, "customGravityX", &customGravityX);
		CheckBsonFieldFloat(iter, "customGravityY", &customGravityY);
		CheckBsonFieldInt(iter, "airMode", &airMode);
		CheckBsonFieldFloat(iter, "ambientAirTemp", &ambientAirTemp);
		CheckBsonFieldInt(iter, "edgeMode", &edgeMode);
		CheckBsonFieldInt(iter, "pmapbits", &pmapbits);
		CheckBsonFieldBool(iter, "ensureDeterminism", &ensureDeterminism);
		CheckBsonFieldLong(iter, "frameCount", reinterpret_cast<int64_t *>(&frameCount));
		CheckBsonFieldLong(iter, "rngState0", reinterpret_cast<int64_t *>(&rngState[0]));
		CheckBsonFieldLong(iter, "rngState1", reinterpret_cast<int64_t *>(&rngState[1]));
		if (!strcmp(bson_iterator_key(&iter), "rngState"))
		{
			if (bson_iterator_type(&iter) == BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter)) == BSON_BIN_USER && bson_iterator_bin_len(&iter) == sizeof(rngState))
			{
				memcpy(&rngState, bson_iterator_bin_data(&iter), sizeof(rngState));
				hasRngState = true;
			}
			else
			{
				fprintf(stderr, "Invalid datatype for rngState: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if (!strcmp(bson_iterator_key(&iter), "signs"))
		{
			if (bson_iterator_type(&iter)==BSON_ARRAY)
			{
				bson_iterator subiter;
				bson_iterator_subiterator(&iter, &subiter);
				while (bson_iterator_next(&subiter))
				{
					if (!strcmp(bson_iterator_key(&subiter), "sign"))
					{
						if (bson_iterator_type(&subiter) == BSON_OBJECT)
						{
							bson_iterator signiter;
							bson_iterator_subiterator(&subiter, &signiter);

							sign tempSign("", 0, 0, sign::Left);
							while (bson_iterator_next(&signiter))
							{
								if (!strcmp(bson_iterator_key(&signiter), "text") && bson_iterator_type(&signiter) == BSON_STRING)
								{
									tempSign.text = format::CleanString(ByteString(bson_iterator_string(&signiter)).FromUtf8(), true, true, true).Substr(0, 45);
									if (version < Version(94, 2))
									{
										if (tempSign.text == "{t}")
										{
											tempSign.text = "Temp: {t}";
										}
										else if (tempSign.text == "{p}")
										{
											tempSign.text = "Pressure: {p}";
										}
									}
								}
								else if (!strcmp(bson_iterator_key(&signiter), "justification") && bson_iterator_type(&signiter) == BSON_INT)
								{
									tempSign.ju = (sign::Justification)bson_iterator_int(&signiter);
								}
								else if (!strcmp(bson_iterator_key(&signiter), "x") && bson_iterator_type(&signiter) == BSON_INT)
								{
									tempSign.x = bson_iterator_int(&signiter)+partP.X;
								}
								else if (!strcmp(bson_iterator_key(&signiter), "y") && bson_iterator_type(&signiter) == BSON_INT)
								{
									tempSign.y = bson_iterator_int(&signiter)+partP.Y;
								}
								else
								{
									fprintf(stderr, "Unknown sign property %s\n", bson_iterator_key(&signiter));
								}
							}
							tempSigns.push_back(tempSign);
						}
						else
						{
							fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&subiter));
						}
					}
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if (!strcmp(bson_iterator_key(&iter), "stkm"))
		{
			if (bson_iterator_type(&iter) == BSON_OBJECT)
			{
				bson_iterator stkmiter;
				bson_iterator_subiterator(&iter, &stkmiter);
				while (bson_iterator_next(&stkmiter))
				{
					CheckBsonFieldBool(stkmiter, "rocketBoots1", &stkm.rocketBoots1);
					CheckBsonFieldBool(stkmiter, "rocketBoots2", &stkm.rocketBoots2);
					CheckBsonFieldBool(stkmiter, "fan1", &stkm.fan1);
					CheckBsonFieldBool(stkmiter, "fan2", &stkm.fan2);
					if (!strcmp(bson_iterator_key(&stkmiter), "rocketBootsFigh") && bson_iterator_type(&stkmiter) == BSON_ARRAY)
					{
						bson_iterator fighiter;
						bson_iterator_subiterator(&stkmiter, &fighiter);
						while (bson_iterator_next(&fighiter))
						{
							if (bson_iterator_type(&fighiter) == BSON_INT)
								stkm.rocketBootsFigh.push_back(bson_iterator_int(&fighiter));
						}
					}
					else if (!strcmp(bson_iterator_key(&stkmiter), "fanFigh") && bson_iterator_type(&stkmiter) == BSON_ARRAY)
					{
						bson_iterator fighiter;
						bson_iterator_subiterator(&stkmiter, &fighiter);
						while (bson_iterator_next(&fighiter))
						{
							if (bson_iterator_type(&fighiter) == BSON_INT)
								stkm.fanFigh.push_back(bson_iterator_int(&fighiter));
						}
					}
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if (!strcmp(bson_iterator_key(&iter), "palette"))
		{
			palette.clear();
			if (bson_iterator_type(&iter) == BSON_ARRAY)
			{
				bson_iterator subiter;
				bson_iterator_subiterator(&iter, &subiter);
				while (bson_iterator_next(&subiter))
				{
					if (bson_iterator_type(&subiter) == BSON_INT)
					{
						ByteString id = bson_iterator_key(&subiter);
						int num = bson_iterator_int(&subiter);
						palette.push_back(PaletteItem(id, num));
					}
				}
			}
		}
		else if (!strcmp(bson_iterator_key(&iter), "minimumVersion"))
		{
			if (bson_iterator_type(&iter) == BSON_OBJECT)
			{
				Version<2> minimumVersion;
				{
					int major = INT_MAX, minor = INT_MAX;
					bson_iterator subiter;
					bson_iterator_subiterator(&iter, &subiter);
					while (bson_iterator_next(&subiter))
					{
						if (bson_iterator_type(&subiter) == BSON_INT)
						{
							if (!strcmp(bson_iterator_key(&subiter), "major"))
								major = bson_iterator_int(&subiter);
							else if (!strcmp(bson_iterator_key(&subiter), "minor"))
								minor = bson_iterator_int(&subiter);
						}
					}
					minimumVersion = Version(major, minor);
				}
				if (effectiveVersion < minimumVersion)
				{
					String errorMessage = String::Build("Save from a newer version: Requires version ", minimumVersion[0], ".", minimumVersion[1]);
					throw ParseException(ParseException::WrongVersion, errorMessage);
				}
				else if (ALLOW_FAKE_NEWER_VERSION && currentVersion < minimumVersion)
				{
					fakeNewerVersion = true;
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if (wantAuthors && !strcmp(bson_iterator_key(&iter), "authors"))
		{
			if (bson_iterator_type(&iter) == BSON_OBJECT)
			{
				// we need to clear authors because the save may be read multiple times in the stamp browser (loading and rendering twice)
				// seems inefficient ...
				authors.clear();
				ConvertBsonToJson(&iter, &authors);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
	}

	auto paletteRemap = [this](auto maxVersion, ByteString from, ByteString to) {
		if (version <= maxVersion)
		{
			auto it = std::find_if(palette.begin(), palette.end(), [&from](auto &item) {
				return item.first == from;
			});
			if (it != palette.end())
			{
				it->first = to;
			}
		}
	};
	paletteRemap(Version(87, 1), "DEFAULT_PT_TUGN", "DEFAULT_PT_TUNG");
	paletteRemap(Version(90, 1), "DEFAULT_PT_REPL", "DEFAULT_PT_RPEL");
	paletteRemap(Version(92, 0), "DEFAULT_PT_E180", "DEFAULT_PT_HEAC");
	paletteRemap(Version(92, 0), "DEFAULT_PT_E181", "DEFAULT_PT_SAWD");
	paletteRemap(Version(92, 0), "DEFAULT_PT_E182", "DEFAULT_PT_POLO");
	paletteRemap(Version(93, 3), "DEFAULT_PT_RAYT", "DEFAULT_PT_LDTC");

	//Read wall and fan data
	if(wallData)
	{
		auto wallDataPlane = PlaneAdapter<PlaneBase<const unsigned char>>(blockS, std::in_place, wallData);
		unsigned int j = 0;
		if (blockS.X * blockS.Y > int(wallDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough wall data");
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			unsigned char bm = 0;
			if (wallDataPlane[bpos])
				bm = wallDataPlane[bpos];

			switch (bm)
			{
			case O_WL_WALLELEC:     bm = WL_WALLELEC;     break;
			case O_WL_EWALL:        bm = WL_EWALL;        break;
			case O_WL_DETECT:       bm = WL_DETECT;       break;
			case O_WL_STREAM:       bm = WL_STREAM;       break;
			case O_WL_FAN:
			case O_WL_FANHELPER:    bm = WL_FAN;          break;
			case O_WL_ALLOWLIQUID:  bm = WL_ALLOWLIQUID;  break;
			case O_WL_DESTROYALL:   bm = WL_DESTROYALL;   break;
			case O_WL_ERASE:        bm = WL_ERASE;        break;
			case O_WL_WALL:         bm = WL_WALL;         break;
			case O_WL_ALLOWAIR:     bm = WL_ALLOWAIR;     break;
			case O_WL_ALLOWSOLID:   bm = WL_ALLOWPOWDER;  break;
			case O_WL_ALLOWALLELEC: bm = WL_ALLOWALLELEC; break;
			case O_WL_EHOLE:        bm = WL_EHOLE;        break;
			case O_WL_ALLOWGAS:     bm = WL_ALLOWGAS;     break;
			case O_WL_GRAV:         bm = WL_GRAV;         break;
			case O_WL_ALLOWENERGY:  bm = WL_ALLOWENERGY;  break;
			}

			if (bm == WL_FAN && fanData)
			{
				if(j+1 >= fanDataLen)
				{
					fprintf(stderr, "Not enough fan data\n");
				}
				fanVelX[blockP + bpos] = (fanData[j++]-127.0f)/64.0f;
				fanVelY[blockP + bpos] = (fanData[j++]-127.0f)/64.0f;
			}

			if (bm >= UI_WALLCOUNT)
				bm = 0;
			blockMap[blockP + bpos] = bm;
		}
	}

	//Read pressure data
	if (pressData)
	{
		unsigned int j = 0;
		unsigned char i, i2;
		if (blockS.X * blockS.Y > int(pressDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough pressure data");
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			i = pressData[j++];
			i2 = pressData[j++];
			pressure[blockP + bpos] = ((i+(i2<<8))/128.0f)-256;
		}
		hasPressure = true;
	}

	//Read vx data
	if (vxData)
	{
		unsigned int j = 0;
		unsigned char i, i2;
		if (blockS.X * blockS.Y > int(vxDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough vx data");
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			i = vxData[j++];
			i2 = vxData[j++];
			velocityX[blockP + bpos] = ((i+(i2<<8))/128.0f)-256;
		}
	}

	//Read vy data
	if (vyData)
	{
		unsigned int j = 0;
		unsigned char i, i2;
		if (blockS.X * blockS.Y > int(vyDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough vy data");
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			i = vyData[j++];
			i2 = vyData[j++];
			velocityY[blockP + bpos] = ((i+(i2<<8))/128.0f)-256;
		}
	}

	//Read ambient data
	if (ambientData)
	{
		unsigned int i = 0, tempTemp;
		if (blockS.X * blockS.Y > int(ambientDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough ambient heat data");
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			tempTemp = ambientData[i++];
			tempTemp |= (((unsigned)ambientData[i++]) << 8);
			ambientHeat[blockP + bpos] = float(tempTemp);
		}
		hasAmbientHeat = true;
	}

	if (blockAirData)
	{
		if (blockS.X * blockS.Y * 2 > int(blockAirDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough block air data");
		auto blockAirDataPlane = PlaneAdapter<PlaneBase<const unsigned char>>(blockS, std::in_place, blockAirData);
		auto blockAirhDataPlane = PlaneAdapter<PlaneBase<const unsigned char>>(blockS, std::in_place, blockAirData + blockS.X * blockS.Y);
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			blockAir [blockP + bpos] = blockAirDataPlane [bpos];
			blockAirh[blockP + bpos] = blockAirhDataPlane[bpos];
		}
		hasBlockAirMaps = true;
	}

	if (gravityData)
	{
		if (blockS.X * blockS.Y * 4 > int(gravityDataLen))
		{
			throw ParseException(ParseException::Corrupt, "Not enough gravity data");
		}
		auto massDataPlane   = PlaneAdapter<PlaneBase<const float   >>(blockS, std::in_place, reinterpret_cast<const float    *>(gravityData                                          ));
		auto maskDataPlane   = PlaneAdapter<PlaneBase<const uint32_t>>(blockS, std::in_place, reinterpret_cast<const uint32_t *>(gravityData +     blockS.X * blockS.Y * sizeof(float)));
		auto forceXDataPlane = PlaneAdapter<PlaneBase<const float   >>(blockS, std::in_place, reinterpret_cast<const float    *>(gravityData + 2 * blockS.X * blockS.Y * sizeof(float)));
		auto forceYDataPlane = PlaneAdapter<PlaneBase<const float   >>(blockS, std::in_place, reinterpret_cast<const float    *>(gravityData + 3 * blockS.X * blockS.Y * sizeof(float)));
		for (auto bpos : blockS.OriginRect().Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
		{
			gravMass  [blockP + bpos] = massDataPlane  [bpos];
			gravMask  [blockP + bpos] = maskDataPlane  [bpos];
			gravForceX[blockP + bpos] = forceXDataPlane[bpos];
			gravForceY[blockP + bpos] = forceYDataPlane[bpos];
		}
		hasGravityMaps = true;
	}

	//Read particle data
	if (partsData && partsPosData)
	{
		int newIndex = 0, tempTemp;
		int posCount, posTotal, partsPosDataIndex = 0;
		if (partS.X * partS.Y * 3 > int(partsPosDataLen))
			throw ParseException(ParseException::Corrupt, "Not enough particle position data");

		partsCount = 0;

		unsigned int i = 0;
		newIndex = 0;
		for (auto pos : RectSized(partP, partS).Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		{
			//Read total number of particles at this position
			posTotal = 0;
			posTotal |= partsPosData[partsPosDataIndex++]<<16;
			posTotal |= partsPosData[partsPosDataIndex++]<<8;
			posTotal |= partsPosData[partsPosDataIndex++];
			//Put the next posTotal particles at this position
			for (posCount = 0; posCount < posTotal; posCount++)
			{
				particlesCount = newIndex+1;
				//i+3 because we have 4 bytes of required fields (type (1), descriptor (2), temp (1))
				if (i+3 >= partsDataLen)
					throw ParseException(ParseException::Corrupt, "Ran past particle data buffer");
				unsigned int fieldDescriptor = (unsigned int)(partsData[i+1]);
				fieldDescriptor |= (unsigned int)(partsData[i+2]) << 8;

				if (newIndex < 0 || newIndex >= NPART)
					throw ParseException(ParseException::Corrupt, "Too many particles");

				//Clear the particle, ready for our new properties
				memset(&(particles[newIndex]), 0, sizeof(Particle));

				//Required fields
				particles[newIndex].type = partsData[i];
				particles[newIndex].x = float(pos.X);
				particles[newIndex].y = float(pos.Y);
				i+=3;

				// Read type (2nd byte)
				if (fieldDescriptor & 0x4000)
					particles[newIndex].type |= (((unsigned)partsData[i++]) << 8);

				//Read temp
				if(fieldDescriptor & 0x01)
				{
					//Full 16bit int
					tempTemp = partsData[i++];
					tempTemp |= (((unsigned)partsData[i++]) << 8);
					particles[newIndex].temp = float(tempTemp);
				}
				else
				{
					//1 Byte room temp offset
					tempTemp = partsData[i++];
					if (tempTemp >= 0x80)
					{
						tempTemp -= 0x100;
					}
					particles[newIndex].temp = tempTemp+294.15f;
				}

				// fieldDesc3
				if (fieldDescriptor & 0x8000)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading third byte of field descriptor");
					fieldDescriptor |= (unsigned int)(partsData[i++]) << 16;
				}

				//Read life
				if(fieldDescriptor & 0x02)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading life");
					particles[newIndex].life = partsData[i++];
					//i++;
					//Read 2nd byte
					if(fieldDescriptor & 0x04)
					{
						if (i >= partsDataLen)
							throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading life");
						particles[newIndex].life |= (((unsigned)partsData[i++]) << 8);
					}
				}

				//Read tmp
				if(fieldDescriptor & 0x08)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading tmp");
					particles[newIndex].tmp = partsData[i++];
					//Read 2nd byte
					if(fieldDescriptor & 0x10)
					{
						if (i >= partsDataLen)
							throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading tmp");
						particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 8);
						//Read 3rd and 4th bytes
						if(fieldDescriptor & 0x1000)
						{
							if (i+1 >= partsDataLen)
								throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading tmp");
							particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 24);
							particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 16);
						}
					}
				}

				//Read ctype
				if(fieldDescriptor & 0x20)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading ctype");
					particles[newIndex].ctype = partsData[i++];
					//Read additional bytes
					if(fieldDescriptor & 0x200)
					{
						if (i+2 >= partsDataLen)
							throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading ctype");
						particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 24);
						particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 16);
						particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 8);
					}
				}

				//Read dcolour
				if(fieldDescriptor & 0x40)
				{
					if (i+3 >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading deco");
					particles[newIndex].dcolour = (((unsigned)partsData[i++]) << 24);
					particles[newIndex].dcolour |= (((unsigned)partsData[i++]) << 16);
					particles[newIndex].dcolour |= (((unsigned)partsData[i++]) << 8);
					particles[newIndex].dcolour |= ((unsigned)partsData[i++]);
				}

				//Read vx
				if(fieldDescriptor & 0x80)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading vx");
					particles[newIndex].vx = (partsData[i++]-127.0f)/16.0f;
				}

				//Read vy
				if(fieldDescriptor & 0x100)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading vy");
					particles[newIndex].vy = (partsData[i++]-127.0f)/16.0f;
				}

				//Read tmp2
				if(fieldDescriptor & 0x400)
				{
					if (i >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading tmp2");
					particles[newIndex].tmp2 = partsData[i++];
					if(fieldDescriptor & 0x800)
					{
						if (i >= partsDataLen)
							throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading tmp2");
						particles[newIndex].tmp2 |= (((unsigned)partsData[i++]) << 8);
					}
				}

				//Read tmp3 and tmp4
				if(fieldDescriptor & 0x2000)
				{
					if (i+3 >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading tmp3 and tmp4");
					if (fieldDescriptor & 0x10000 && i+7 >= partsDataLen)
						throw ParseException(ParseException::Corrupt, "Ran past particle data buffer while loading high halves of tmp3 and tmp4");
					unsigned int tmp34;
					tmp34  = (unsigned int)partsData[i + 0];
					tmp34 |= (unsigned int)partsData[i + 1] << 8;
					if (fieldDescriptor & 0x10000)
					{
						tmp34 |= (unsigned int)partsData[i + 4] << 16;
						tmp34 |= (unsigned int)partsData[i + 5] << 24;
					}
					particles[newIndex].tmp3 = int(tmp34);
					tmp34  = (unsigned int)partsData[i + 2];
					tmp34 |= (unsigned int)partsData[i + 3] << 8;
					if (fieldDescriptor & 0x10000)
					{
						tmp34 |= (unsigned int)partsData[i + 6] << 16;
						tmp34 |= (unsigned int)partsData[i + 7] << 24;
					}
					particles[newIndex].tmp4 = int(tmp34);
					i += 4;
					if (fieldDescriptor & 0x10000)
						i += 4;
				}

				//Particle specific parsing:
				switch(particles[newIndex].type)
				{
				case PT_SOAP:
					//Clear soap links, links will be added back in if soapLinkData is present
					particles[newIndex].ctype &= ~6;
					break;
				case PT_BOMB:
					if (particles[newIndex].tmp!=0 && savedVersion < 81)
					{
						particles[newIndex].type = PT_EMBR;
						particles[newIndex].ctype = 0;
						if (particles[newIndex].tmp==1)
							particles[newIndex].tmp = 0;
					}
					break;
				case PT_DUST:
					if (particles[newIndex].life>0 && savedVersion < 81)
					{
						particles[newIndex].type = PT_EMBR;
						particles[newIndex].ctype = (particles[newIndex].tmp2<<16) | (particles[newIndex].tmp<<8) | particles[newIndex].ctype;
						particles[newIndex].tmp = 1;
					}
					break;
				case PT_FIRW:
					if (particles[newIndex].tmp>=2 && savedVersion < 81)
					{
						particles[newIndex].type = PT_EMBR;
						particles[newIndex].ctype = Renderer::firwTableAt(particles[newIndex].tmp - 4).Pack();
						particles[newIndex].tmp = 1;
					}
					break;
				case PT_PSTN:
					if (savedVersion < 87 && particles[newIndex].ctype)
						particles[newIndex].life = 1;
					if (savedVersion < 91)
						particles[newIndex].temp = 283.15f;
					break;
				case PT_FILT:
					if (savedVersion < 89)
					{
						if (particles[newIndex].tmp<0 || particles[newIndex].tmp>3)
							particles[newIndex].tmp = 6;
						particles[newIndex].ctype = 0;
					}
					break;
				case PT_QRTZ:
				case PT_PQRT:
					if (savedVersion < 89)
					{
						particles[newIndex].tmp2 = particles[newIndex].tmp;
						particles[newIndex].tmp = particles[newIndex].ctype;
						particles[newIndex].ctype = 0;
					}
					break;
				case PT_PHOT:
					if (savedVersion < 90)
					{
						particles[newIndex].flags |= FLAG_PHOTDECO;
					}
					break;
				case PT_VINE:
					if (savedVersion < 91)
					{
						particles[newIndex].tmp = 1;
					}
					break;
				case PT_DLAY:
					// correct DLAY temperature in older saves
					// due to either the +.5f now done in DLAY (higher temps), or rounding errors in the old DLAY code (room temperature temps),
					// the delay in all DLAY from older versions will always be one greater than it should
					if (savedVersion < 91)
					{
						particles[newIndex].temp = particles[newIndex].temp - 1.0f;
					}
					break;
				case PT_CRAY:
					if (savedVersion < 91)
					{
						if (particles[newIndex].tmp2)
						{
							particles[newIndex].ctype |= particles[newIndex].tmp2<<8;
							particles[newIndex].tmp2 = 0;
						}
					}
					break;
				case PT_CONV:
					if (savedVersion < 91)
					{
						if (particles[newIndex].tmp)
						{
							particles[newIndex].ctype |= particles[newIndex].tmp<<8;
							particles[newIndex].tmp = 0;
						}
					}
					break;
				case PT_PIPE:
				case PT_PPIP:
					if (savedVersion < 93 && !fakeNewerVersion)
					{
						if (particles[newIndex].ctype == 1)
							particles[newIndex].tmp |= 0x00020000; //PFLAG_INITIALIZING
						particles[newIndex].tmp |= (particles[newIndex].ctype-1)<<18;
						particles[newIndex].ctype = particles[newIndex].tmp&0xFF;
					}
					break;
				case PT_TSNS:
				case PT_HSWC:
				case PT_PSNS:
				case PT_PUMP:
					if (savedVersion < 93 && !fakeNewerVersion)
					{
						particles[newIndex].tmp = 0;
					}
					break;
				case PT_LIFE:
					if (savedVersion < 96 && !fakeNewerVersion)
					{
						if (particles[newIndex].ctype >= 0 && particles[newIndex].ctype < NGOL)
						{
							particles[newIndex].tmp2 = particles[newIndex].tmp;
							if (!particles[newIndex].dcolour)
								particles[newIndex].dcolour = builtinGol[particles[newIndex].ctype].colour.Pack();
							particles[newIndex].tmp = builtinGol[particles[newIndex].ctype].colour2.Pack();
						}
					}
				}
				if (PressureInTmp3(particles[newIndex].type))
				{
					// pavg[1] used to be saved as a u16, which PressureInTmp3 elements then treated as
					// an i16. tmp3 is now saved as a u32, or as a u16 if it's small enough. PressureInTmp3
					// elements will never use the upper 16 bits, and should still treat the lower 16 bits
					// as an i16, so they need sign extension.
					auto tmp3 = (unsigned int)(particles[newIndex].tmp3);
					if (tmp3 & 0x8000U)
					{
						tmp3 |= 0xFFFF0000U;
						particles[newIndex].tmp3 = int(tmp3);
					}
				}
				//note: PSv was used in version 77.0 and every version before, add something in PSv too if the element is that old
				newIndex++;
				partsCount++;
			}
		}

		if (i != partsDataLen)
			throw ParseException(ParseException::Corrupt, "Didn't reach end of particle data buffer");
	}

	if (soapLinkData)
	{
		unsigned int soapLinkDataPos = 0;
		for (unsigned int i = 0; i < partsCount; i++)
		{
			if (particles[i].type == PT_SOAP)
			{
				// Get the index of the particle forward linked from this one, if present in the save data
				unsigned int linkedIndex = 0;
				if (soapLinkDataPos+3 > soapLinkDataLen) break;
				linkedIndex |= soapLinkData[soapLinkDataPos++]<<16;
				linkedIndex |= soapLinkData[soapLinkDataPos++]<<8;
				linkedIndex |= soapLinkData[soapLinkDataPos++];
				// All indexes in soapLinkData and partsSimIndex have 1 added to them (0 means not saved/loaded)
				if (!linkedIndex || linkedIndex-1 >= partsCount)
					continue;
				linkedIndex = linkedIndex-1;

				//Attach the two particles
				particles[i].ctype |= 2;
				particles[i].tmp = linkedIndex;
				particles[linkedIndex].ctype |= 4;
				particles[linkedIndex].tmp2 = i;
			}
		}
	}

	if (tempSigns.size())
	{
		for (size_t i = 0; i < tempSigns.size(); i++)
		{
			if(signs.size() == MAXSIGNS)
				break;
			signs.push_back(tempSigns[i]);
		}
	}
}

#define MTOS_EXPAND(str) #str
#define MTOS(str) MTOS_EXPAND(str)
void GameSave::readPSv(const std::vector<char> &dataVec)
{
	auto &builtinGol = SimulationData::builtinGol;
	Renderer::PopulateTables();

	auto *saveData = reinterpret_cast<const unsigned char *>(dataVec.data());
	auto dataLength = int(dataVec.size());
	int q,p=0, pty, ty, legacy_beta=0;
	Vec2<int> blockP = { 0, 0 };
	int new_format = 0, ttv = 0;

	std::vector<sign> tempSigns;
	char tempSignText[255];
	sign tempSign("", 0, 0, sign::Left);

	auto &builtinElements = GetElements();

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	if (dataLength<16)
		throw ParseException(ParseException::Corrupt, "No save data");
	if (!(saveData[2]==0x43 && saveData[1]==0x75 && saveData[0]==0x66) && !(saveData[2]==0x76 && saveData[1]==0x53 && saveData[0]==0x50))
		throw ParseException(ParseException::Corrupt, "Unknown format");
	if (saveData[2]==0x76 && saveData[1]==0x53 && saveData[0]==0x50) {
		new_format = 1;
	}
	if (saveData[4]>97) // this used to respect currentVersion but no valid PSv will ever have a version > 97 so it's ok to hardcode
		throw ParseException(ParseException::WrongVersion, "Save from newer version");
	version = { saveData[4], 0 };
	auto ver = version[0];

	if (ver<34)
	{
		legacyEnable = 1;
	}
	else
	{
		if (ver>=44) {
			legacyEnable = saveData[3]&0x01;
			paused = (saveData[3]>>1)&0x01;
			if (ver>=46) {
				gravityMode = ((saveData[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				airMode = ((saveData[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49) {
				gravityEnable = ((saveData[3]>>7)&0x01);
			}
		} else {
			if (saveData[3]==1||saveData[3]==0) {
				legacyEnable = saveData[3];
			} else {
				legacy_beta = 1;
			}
		}
	}

	auto blockS = Vec2{ int(saveData[6]), int(saveData[7]) };
	blockP = blockP.Clamp(blockS.OriginRect());

	if (saveData[5]!=CELL || blockP.X+blockS.X>XCELLS || blockP.Y+blockS.Y>YCELLS)
		throw ParseException(ParseException::InvalidDimensions, "Save too large");
	int size = (unsigned)saveData[8];
	size |= ((unsigned)saveData[9])<<8;
	size |= ((unsigned)saveData[10])<<16;
	size |= ((unsigned)saveData[11])<<24;

	if (size > 209715200 || !size)
		throw ParseException(ParseException::InvalidDimensions, "Save data too large");

	std::vector<char> bsonData;
	switch (auto status = BZ2WDecompress(bsonData, std::span(reinterpret_cast<const char *>(saveData + 12), dataLength - 12), size))
	{
	case BZ2WDecompressOk: break;
	case BZ2WDecompressNomem: throw ParseException(ParseException::Corrupt, "Cannot allocate memory");
	default: throw ParseException(ParseException::Corrupt, String::Build("Cannot decompress: status ", int(status)));
	}

	setSize(blockS);
	const auto *data = reinterpret_cast<const unsigned char *>(bsonData.data());
	dataLength = bsonData.size();

	if constexpr (DEBUG)
	{
		std::cout << "Parsing " << dataLength << " bytes of data, version " << ver << std::endl;
	}

	if (dataLength < blockS.X*blockS.Y)
		throw ParseException(ParseException::Corrupt, "Save data corrupt (missing data)");

	// normalize coordinates
	auto partS = blockS * CELL;
	auto partP = blockP * CELL;

	if (ver<46) {
		gravityMode = GRAV_VERTICAL;
		airMode = AIR_ON;
	}

	PlaneAdapter<std::vector<int>> particleIDMap(RES, 0);

	// load the required air state
	for (auto bpos : RectSized(blockP, blockS).Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		if (data[p])
		{
			//In old saves, ignore walls created by sign tool bug
			//Not ignoring other invalid walls or invalid walls in new saves, so that any other bugs causing them are easier to notice, find and fix
			if (ver>=44 && ver<71 && data[p]==O_WL_SIGN)
			{
				p++;
				continue;
			}
			auto bm = data[p];
			switch (bm)
			{
			case  1: bm = WL_WALL        ; break;
			case  2: bm = WL_DESTROYALL  ; break;
			case  3: bm = WL_ALLOWLIQUID ; break;
			case  4: bm = WL_FAN         ; break;
			case  5: bm = WL_STREAM      ; break;
			case  6: bm = WL_DETECT      ; break;
			case  7: bm = WL_EWALL       ; break;
			case  8: bm = WL_WALLELEC    ; break;
			case  9: bm = WL_ALLOWAIR    ; break;
			case 10: bm = WL_ALLOWPOWDER ; break;
			case 11: bm = WL_ALLOWALLELEC; break;
			case 12: bm = WL_EHOLE       ; break;
			case 13: bm = WL_ALLOWGAS    ; break;
			}

			if (ver>=44)
			{
				/* The numbers used to save walls were changed, starting in v44.
				 * The new numbers are ignored for older versions due to some corruption of bmap in saves from older versions.
				 */
				switch (bm)
				{
				case O_WL_WALLELEC:     bm = WL_WALLELEC    ; break;
				case O_WL_EWALL:        bm = WL_EWALL       ; break;
				case O_WL_DETECT:       bm = WL_DETECT      ; break;
				case O_WL_STREAM:       bm = WL_STREAM      ; break;
				case O_WL_FAN:
				case O_WL_FANHELPER:    bm = WL_FAN         ; break;
				case O_WL_ALLOWLIQUID:  bm = WL_ALLOWLIQUID ; break;
				case O_WL_DESTROYALL:   bm = WL_DESTROYALL  ; break;
				case O_WL_ERASE:        bm = WL_ERASE       ; break;
				case O_WL_WALL:         bm = WL_WALL        ; break;
				case O_WL_ALLOWAIR:     bm = WL_ALLOWAIR    ; break;
				case O_WL_ALLOWSOLID:   bm = WL_ALLOWPOWDER ; break;
				case O_WL_ALLOWALLELEC: bm = WL_ALLOWALLELEC; break;
				case O_WL_EHOLE:        bm = WL_EHOLE       ; break;
				case O_WL_ALLOWGAS:     bm = WL_ALLOWGAS    ; break;
				case O_WL_GRAV:         bm = WL_GRAV        ; break;
				case O_WL_ALLOWENERGY:  bm = WL_ALLOWENERGY ; break;
				}
			}

			if (bm >= UI_WALLCOUNT)
				bm = 0;
			blockMap[bpos] = bm;
		}

		p++;
	}
	for (auto bpos : RectSized(blockP, blockS).Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto dataPlane = PlaneAdapter<PlaneBase<const unsigned char>>(blockS, std::in_place, data);
		if (dataPlane[bpos - blockP]==4||(ver>=44 && dataPlane[bpos - blockP]==O_WL_FAN))
		{
			if (p >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			fanVelX[bpos] = (data[p++]-127.0f)/64.0f;
		}
	}
	for (auto bpos : RectSized(blockP, blockS).Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto dataPlane = PlaneAdapter<PlaneBase<const unsigned char>>(blockS, std::in_place, data);
		if (dataPlane[bpos - blockP]==4||(ver>=44 && dataPlane[bpos - blockP]==O_WL_FAN))
		{
			if (p >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			fanVelY[bpos] = (data[p++]-127.0f)/64.0f;
		}
	}

	// load the particle map
	{
		auto k = 0;
		pty = p;
		for (auto pos : RectSized(partP, partS).Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		{
			if (p >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			auto j=data[p++];
			if (int(j) >= PT_NUM) { // not possible these days since PMAPBITS >= 8
				j = PT_DUST;//throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			}
			if (j)
			{
				memset(&particles[k], 0, sizeof(Particle));
				particles[k].type = j;
				if (j == PT_COAL)
					particles[k].tmp = 50;
				if (j == PT_FUSE)
					particles[k].tmp = 50;
				if (j == PT_PHOT)
					particles[k].ctype = 0x3fffffff;
				if (j == PT_SOAP)
					particles[k].ctype = 0;
				if (j==PT_BIZR || j==PT_BIZRG || j==PT_BIZRS)
					particles[k].ctype = 0x47FFFF;
				particles[k].x = (float)pos.X;
				particles[k].y = (float)pos.Y;
				particleIDMap[pos - partP] = k+1;
				particlesCount = ++k;
			}
		}
	}

	// load particle properties
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		if (i)
		{
			i--;
			if (p+1 >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			if (i < NPART)
			{
				particles[i].vx = (data[p++]-127.0f)/16.0f;
				particles[i].vy = (data[p++]-127.0f)/16.0f;
			}
			else
				p += 2;
		}
	}
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		if (i)
		{
			if (ver>=44) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					ttv = (data[p++])<<8;
					ttv |= (data[p++]);
					particles[i-1].life = ttv;
				} else {
					p+=2;
				}
			} else {
				if (p >= dataLength)
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				if (i <= NPART)
					particles[i-1].life = data[p++]*4;
				else
					p++;
			}
		}
	}
	if (ver>=44) {
		for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		{
			auto i = particleIDMap[pos];
			if (i)
			{
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					ttv = (data[p++])<<8;
					ttv |= (data[p++]);
					particles[i-1].tmp = ttv;
					if (ver<53 && !particles[i-1].tmp)
						for (q = 0; q < NGOL; q++) {
							if (particles[i-1].type==builtinGol[q].oldtype && (builtinGol[q].ruleset >> 17)==0)
								particles[i-1].tmp = (builtinGol[q].ruleset >> 17)+1;
						}
					if (ver>=51 && ver<53 && particles[i-1].type==PT_PBCN)
					{
						particles[i-1].tmp2 = particles[i-1].tmp;
						particles[i-1].tmp = 0;
					}
				} else {
					p+=2;
				}
			}
		}
	}
	auto dataPlanePty = PlaneAdapter<PlaneBase<const unsigned char>>(partS, std::in_place, data + pty);
	if (ver>=53) {
		for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		{
			auto i = particleIDMap[pos];
			ty = dataPlanePty[pos];
			if (i && (ty==PT_PBCN || (ty==PT_TRON && ver>=77)))
			{
				if (p >= dataLength)
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				if (i <= NPART)
					particles[i-1].tmp2 = data[p++];
				else
					p++;
			}
		}
	}
	//Read ALPHA component
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour = data[p++]<<24;
				} else {
					p++;
				}
			}
		}
	}
	//Read RED component
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= data[p++]<<16;
				} else {
					p++;
				}
			}
		}
	}
	//Read GREEN component
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= data[p++]<<8;
				} else {
					p++;
				}
			}
		}
	}
	//Read BLUE component
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= data[p++];
				} else {
					p++;
				}
			}
		}
	}
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		ty = dataPlanePty[pos];
		if (i)
		{
			if (ver>=34&&legacy_beta==0)
			{
				if (p >= dataLength)
				{
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART)
				{
					if (ver>=42) {
						if (new_format) {
							ttv = (data[p++])<<8;
							ttv |= (data[p++]);
							if (particles[i-1].type==PT_PUMP) {
								particles[i-1].temp = ttv + 0.15;//fix PUMP saved at 0, so that it loads at 0.
							} else {
								particles[i-1].temp = float(ttv);
							}
						} else {
							particles[i-1].temp = float((data[p++]*((MAX_TEMP+(-MIN_TEMP))/255))+MIN_TEMP);
						}
					} else {
						particles[i-1].temp = float(((data[p++]*((O_MAX_TEMP+(-O_MIN_TEMP))/255))+O_MIN_TEMP)+273);
					}
				}
				else
				{
					p++;
					if (new_format) {
						p++;
					}
				}
			}
			else
			{
				particles[i-1].temp = builtinElements[particles[i-1].type].DefaultProperties.temp;
			}
		}
	}
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		auto i = particleIDMap[pos];
		int gnum = 0;
		ty = dataPlanePty[pos];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43) || (ty==PT_LIFE && ver>=51) || (ty==PT_PBCN && ver>=52) || (ty==PT_WIRE && ver>=55) || (ty==PT_STOR && ver>=59) || (ty==PT_CONV && ver>=60)))
		{
			if (p >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			if (i <= NPART)
				particles[i-1].ctype = data[p++];
			else
				p++;
		}
		// no more particle properties to load, so we can change type here without messing up loading
		if (i && i<=NPART)
		{
			if (ver<90 && particles[i-1].type == PT_PHOT)
			{
				particles[i-1].flags |= FLAG_PHOTDECO;
			}
			if (ver<79 && particles[i-1].type == PT_SPNG)
			{
				if (fabs(particles[i-1].vx)>0.0f || fabs(particles[i-1].vy)>0.0f)
					particles[i-1].flags |= FLAG_MOVABLE;
			}

			if (ver<48 && (ty==OLD_PT_WIND || (ty==PT_BRAY&&particles[i-1].life==0)))
			{
				// Replace invisible particles with something sensible and add decoration to hide it
				particles[i-1].dcolour = 0xFF000000;
				particles[i-1].type = PT_DMND;
			}
			if(ver<51 && ((ty>=78 && ty<=89) || (ty>=134 && ty<=146 && ty!=141))){
				//Replace old GOL
				particles[i-1].type = PT_LIFE;
				for (gnum = 0; gnum<NGOL; gnum++){
					if (ty==builtinGol[gnum].oldtype)
						particles[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOL; gnum++){
					if (particles[i-1].ctype==builtinGol[gnum].oldtype)
					{
						particles[i-1].ctype = PT_LIFE;
						particles[i-1].tmp = gnum;
					}
				}
			}
			if (particles[i-1].type == PT_LIFE)
			{
				particles[i-1].tmp2 = particles[i-1].tmp;
				particles[i-1].tmp = 0;
				if (particles[i-1].ctype >= 0 && particles[i-1].ctype < NGOL)
				{
					if (!particles[i-1].dcolour)
						particles[i-1].dcolour = builtinGol[particles[i-1].ctype].colour.Pack();
					particles[i-1].tmp = builtinGol[particles[i-1].ctype].colour2.Pack();
				}
			}
			if(ty==PT_LCRY){
				if(ver<67)
				{
					//New LCRY uses TMP not life
					if(particles[i-1].life>=10)
					{
						particles[i-1].life = 10;
						particles[i-1].tmp2 = 10;
						particles[i-1].tmp = 3;
					}
					else if(particles[i-1].life<=0)
					{
						particles[i-1].life = 0;
						particles[i-1].tmp2 = 0;
						particles[i-1].tmp = 0;
					}
					else if(particles[i-1].life < 10 && particles[i-1].life > 0)
					{
						particles[i-1].tmp = 1;
					}
				}
				else
				{
					particles[i-1].tmp2 = particles[i-1].life;
				}
			}

			if (ver<81)
			{
				if (particles[i-1].type==PT_BOMB && particles[i-1].tmp!=0)
				{
					particles[i-1].type = PT_EMBR;
					particles[i-1].ctype = 0;
					if (particles[i-1].tmp==1)
						particles[i-1].tmp = 0;
				}
				if (particles[i-1].type==PT_DUST && particles[i-1].life>0)
				{
					particles[i-1].type = PT_EMBR;
					particles[i-1].ctype = (particles[i-1].tmp2<<16) | (particles[i-1].tmp<<8) | particles[i-1].ctype;
					particles[i-1].tmp = 1;
				}
				if (particles[i-1].type==PT_FIRW && particles[i-1].tmp>=2)
				{
					particles[i-1].type = PT_EMBR;
					particles[i-1].ctype = Renderer::firwTableAt(particles[i-1].tmp-4).Pack();
					particles[i-1].tmp = 1;
				}
			}
			if (ver < 89)
			{
				if (particles[i-1].type == PT_FILT)
				{
					if (particles[i-1].tmp<0 || particles[i-1].tmp>3)
						particles[i-1].tmp = 6;
					particles[i-1].ctype = 0;
				}
				else if (particles[i-1].type == PT_QRTZ || particles[i-1].type == PT_PQRT)
				{
					particles[i-1].tmp2 = particles[i-1].tmp;
					particles[i-1].tmp = particles[i-1].ctype;
					particles[i-1].ctype = 0;
				}
			}
			if (ver < 91)
			{
				if (particles[i-1].type == PT_VINE)
					particles[i-1].tmp = 1;
				else if (particles[i-1].type == PT_CONV)
				{
					if (particles[i-1].tmp)
					{
						particles[i-1].ctype |= particles[i-1].tmp<<8;
						particles[i-1].tmp = 0;
					}
				}
			}
			if (ver < 93)
			{
				if (particles[i-1].type == PT_PIPE || particles[i-1].type == PT_PPIP)
				{
					if (particles[i-1].ctype == 1)
						particles[i-1].tmp |= 0x00020000; //PFLAG_INITIALIZING
					particles[i-1].tmp |= (particles[i-1].ctype-1)<<18;
					particles[i-1].ctype = particles[i-1].tmp&0xFF;
				}
				else if (particles[i-1].type == PT_HSWC || particles[i-1].type == PT_PUMP)
				{
					particles[i-1].tmp = 0;
				}
			}
		}
	}

	if (p == dataLength) // no sign data, "version 1" PSv
		return;

	auto signCount = data[p++];
	for (auto i = 0; i < signCount; i++)
	{
		if (p+6 > dataLength)
			throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
		{
			int x = data[p++];
			x |= ((unsigned)data[p++])<<8;
			tempSign.x = x+partP.X;
		}
		{
			int y = data[p++];
			y |= ((unsigned)data[p++])<<8;
			tempSign.y = y+partP.Y;
		}
		{
			int ju = data[p++];
			tempSign.ju = (sign::Justification)ju;
		}
		{
			int l = data[p++];
			if (p+l > dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			if(l>254)
				l = 254;
			memcpy(tempSignText, &data[p], l);
			tempSignText[l] = 0;
			p += l;
		}
		tempSign.text = format::CleanString(ByteString(tempSignText).FromUtf8(), true, true, true).Substr(0, 45);
		if (tempSign.text == "{t}")
		{
			tempSign.text = "Temp: {t}";
		}
		else if (tempSign.text == "{p}")
		{
			tempSign.text = "Pressure: {p}";
		}
		tempSigns.push_back(tempSign);
	}

	for (size_t i = 0; i < tempSigns.size(); i++)
	{
		if(signs.size() == MAXSIGNS)
			break;
		signs.push_back(tempSigns[i]);
	}
}
#undef MTOS
#undef MTOS_EXPAND

std::pair<bool, std::vector<char>> GameSave::serialiseOPS() const
{
	if (blockSize.X > 255 || blockSize.Y > 255)
	{
		throw BuildException("simulation size not supported by the save format");
	}

	// minimum version this save is compatible with
	// when building, this number may be increased depending on what elements are used
	// or what properties are detected
	auto minimumVersion = Version(90, 2);
	auto RESTRICTVERSION = [&minimumVersion](auto major, auto minor = 0) {
		// restrict the minimum version this save can be opened with
		auto version = Version(major, minor);
		if (minimumVersion < version)
		{
			minimumVersion = version;
		}
	};

	//Get coords in blocks
	auto blockP = Vec2{ 0, 0 };

	//Snap full coords to block size
	auto partP = blockP * CELL;

	//Original size + offset of original corner from snapped corner, rounded up by adding CELL-1
	auto blockS = blockSize;
	auto partS = blockS * CELL;

	// Copy fan and wall data
	PlaneAdapter<std::vector<unsigned char>> wallData(blockSize);
	bool hasWallData = false;
	std::vector<unsigned char> fanData(blockSize.X*blockSize.Y*2);
	std::vector<unsigned char> pressData(blockSize.X*blockSize.Y*2);
	std::vector<unsigned char> vxData(blockSize.X*blockSize.Y*2);
	std::vector<unsigned char> vyData(blockSize.X*blockSize.Y*2);
	std::vector<unsigned char> ambientData(blockSize.X*blockSize.Y*2, 0);

	std::vector<unsigned char> blockAirData(blockSize.X * blockSize.Y * 2);
	PlaneAdapter<PlaneBase<unsigned char>> blockAirDataPlane (blockSize, std::in_place, blockAirData.data()                            );
	PlaneAdapter<PlaneBase<unsigned char>> blockAirhDataPlane(blockSize, std::in_place, blockAirData.data() + blockSize.X * blockSize.Y);

	std::vector<unsigned char> gravityData(blockSize.X * blockSize.Y * 4 * sizeof(float));
	PlaneAdapter<PlaneBase<float   >> massDataPlane  (blockSize, std::in_place, reinterpret_cast<float    *>(gravityData.data()                                                ));
	PlaneAdapter<PlaneBase<uint32_t>> maskDataPlane  (blockSize, std::in_place, reinterpret_cast<uint32_t *>(gravityData.data() +     blockSize.X * blockSize.Y * sizeof(float)));
	PlaneAdapter<PlaneBase<float   >> forceXDataPlane(blockSize, std::in_place, reinterpret_cast<float    *>(gravityData.data() + 2 * blockSize.X * blockSize.Y * sizeof(float)));
	PlaneAdapter<PlaneBase<float   >> forceYDataPlane(blockSize, std::in_place, reinterpret_cast<float    *>(gravityData.data() + 3 * blockSize.X * blockSize.Y * sizeof(float)));

	unsigned int wallDataLen = blockSize.X*blockSize.Y, fanDataLen = 0, pressDataLen = 0, vxDataLen = 0, vyDataLen = 0, ambientDataLen = 0;

	for (auto bpos : RectSized(blockP, blockS).Range<LEFT_TO_RIGHT, TOP_TO_BOTTOM>())
	{
		wallData[bpos - blockP] = blockMap[bpos];
		if (blockMap[bpos])
			hasWallData = true;

		if (hasPressure)
		{
			//save pressure and x/y velocity grids
			float pres = std::max(-255.0f,std::min(255.0f,pressure[bpos]))+256.0f;
			float velX = std::max(-255.0f,std::min(255.0f,velocityX[bpos]))+256.0f;
			float velY = std::max(-255.0f,std::min(255.0f,velocityY[bpos]))+256.0f;
			pressData[pressDataLen++] = (unsigned char)((int)(pres*128)&0xFF);
			pressData[pressDataLen++] = (unsigned char)((int)(pres*128)>>8);

			vxData[vxDataLen++] = (unsigned char)((int)(velX*128)&0xFF);
			vxData[vxDataLen++] = (unsigned char)((int)(velX*128)>>8);

			vyData[vyDataLen++] = (unsigned char)((int)(velY*128)&0xFF);
			vyData[vyDataLen++] = (unsigned char)((int)(velY*128)>>8);

			blockAirDataPlane [bpos - blockP] = blockAir [bpos];
			blockAirhDataPlane[bpos - blockP] = blockAirh[bpos];

			massDataPlane  [bpos - blockP] = gravMass  [bpos];
			maskDataPlane  [bpos - blockP] = gravMask  [bpos];
			forceXDataPlane[bpos - blockP] = gravForceX[bpos];
			forceYDataPlane[bpos - blockP] = gravForceY[bpos];
		}

		if (hasAmbientHeat)
		{
			int tempTemp = (int)(ambientHeat[bpos]+0.5f);
			ambientData[ambientDataLen++] = tempTemp;
			ambientData[ambientDataLen++] = tempTemp >> 8;
		}

		if (blockMap[bpos] == WL_FAN)
		{
			{
				auto i = (int)(fanVelX[bpos]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
			}
			{
				auto i = (int)(fanVelY[bpos]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
			}
		}
		else if (blockMap[bpos] == WL_STASIS)
		{
			RESTRICTVERSION(94, 0);
		}
	}

	//Index positions of all particles, using linked lists
	//partsPosFirstMap is pmap for the first particle in each position
	//partsPosLastMap is pmap for the last particle in each position
	//partsPosCount is the number of particles in each position
	//partsPosLink contains, for each particle, (i<<8)|1 of the next particle in the same position
	PlaneAdapter<std::vector<unsigned>> partsPosFirstMap(partS, 0);
	PlaneAdapter<std::vector<unsigned>> partsPosLastMap(partS, 0);
	PlaneAdapter<std::vector<unsigned>> partsPosCount(partS, 0);
	std::vector<unsigned> partsPosLink(NPART, 0);
	unsigned int soapCount = 0;
	for(auto i = 0; i < particlesCount; i++)
	{
		if(particles[i].type)
		{
			auto pos = Vec2{ (int)(particles[i].x+0.5f), (int)(particles[i].y+0.5f) };
			//Coordinates relative to top left corner of saved area
			if (!partsPosFirstMap[pos - partP])
			{
				//First entry in list
				partsPosFirstMap[pos - partP] = (i<<8)|1;
				partsPosLastMap[pos - partP] = (i<<8)|1;
			}
			else
			{
				//Add to end of list
				partsPosLink[partsPosLastMap[pos - partP]>>8] = (i<<8)|1;//link to current end of list
				partsPosLastMap[pos - partP] = (i<<8)|1;//set as new end of list
			}
			partsPosCount[pos - partP]++;
		}
	}

	//Store number of particles in each position
	std::vector<unsigned char> partsPosData(partS.X * partS.Y * 3);
	unsigned int partsPosDataLen = 0;
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		unsigned int posCount = partsPosCount[pos];
		partsPosData[partsPosDataLen++] = (posCount&0x00FF0000)>>16;
		partsPosData[partsPosDataLen++] = (posCount&0x0000FF00)>>8;
		partsPosData[partsPosDataLen++] = (posCount&0x000000FF);
	}

	//Copy parts data
	/* Field descriptor [1+2] format:
	 |      15      |      14       |      13       |      12       |      11       |      10       |       9       |       8       |       7       |       6       |       5       |       4       |       3       |       2       |       1       |       0       |
	 |  fieldDesc3  |    type[2]    |  tmp3/4[1+2]  |   tmp[3+4]    |   tmp2[2]     |     tmp2      |   ctype[2]    |      vy       |      vx       |  decorations  |   ctype[1]    |    tmp[2]     |    tmp[1]     |    life[2]    |    life[1]    | temp dbl len  |
	 life[2] means a second byte (for a 16 bit field) if life[1] is present
	 fieldDesc3 means Field descriptor [3] exists
	   Field descriptor [3] format:
	 |      23      |      22       |      21       |      20       |      19       |      18       |      17       |      16       |
	 |   RESERVED   |     FREE      |     FREE      |     FREE      |     FREE      |     FREE      |     FREE      |  tmp3/4[3+4]  |
	 last bit is reserved. If necessary, use it to signify that fieldDescriptor will have another byte
	 That way, if we ever need a 25th bit, we won't have to change the save format
	 */

	auto &builtinElements = GetElements();
	auto &possiblyCarriesType = Particle::PossiblyCarriesType();
	auto &properties = Particle::GetProperties();
	// Allocate enough space to store all Particles and 3 bytes on top of that per Particle, for the field descriptors.
	// In practice, a Particle will never need as much space in the save as in memory; this is just an upper bound to simplify allocation.
	std::vector<unsigned char> partsData(NPART * (sizeof(Particle)+3));
	unsigned int partsDataLen = 0;
	std::vector<unsigned> partsSaveIndex(NPART);
	unsigned int partsCount = 0;
	std::fill(partsSaveIndex.data(), partsSaveIndex.data() + NPART, 0);
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	std::set<int> paletteSet;
	for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
	{
		//Find the first particle in this position
		auto i = partsPosFirstMap[pos];

		//Loop while there is a pmap entry
		while (i)
		{
			unsigned int fieldDesc = 0;
			int tempTemp, vTemp;

			//Turn pmap entry into a particles index
			i = i>>8;

			//Store saved particle index+1 for this partsptr index (0 means not saved)
			partsSaveIndex[i] = (partsCount++) + 1;

			auto part = particles[i];
			paletteSet.insert(part.type);
			for (auto index : possiblyCarriesType)
			{
				if (elements[part.type].CarriesTypeIn & (1U << index))
				{
					auto *prop = reinterpret_cast<int *>(reinterpret_cast<char *>(&part) + properties[index].Offset);
					if (sd.IsElement(TYP(*prop)))
					{
						paletteSet.insert(TYP(*prop));
					}
					else
					{
						*prop = PMAP(ID(*prop), 0);
					}
				}
			}

			//Type (required)
			partsData[partsDataLen++] = part.type;

			//Location of the field descriptor
			int fieldDesc3Loc = 0;
			int fieldDescLoc = partsDataLen++;
			partsDataLen++;

			auto tmp3 = (unsigned int)(part.tmp3);
			auto tmp4 = (unsigned int)(part.tmp4);
			if ((tmp3 || tmp4) && (!PressureInTmp3(part.type) || hasPressure))
			{
				fieldDesc |= 1 << 13;
				// The tmp3 of PressureInTmp3 elements is okay to truncate because the loading code
				// sign extends it anyway, expecting the value to not be higher in magnitude than
				// 256 (max pressure value) * 64 (tmp3 multiplicative bias).
				if (((tmp3 >> 16) || (tmp4 >> 16)) && !PressureInTmp3(part.type))
				{
					fieldDesc |= 1 << 15;
					fieldDesc |= 1 << 16;
					RESTRICTVERSION(97, 0);
				}
			}

			// Extra type byte if necessary
			if (part.type & 0xFF00)
			{
				partsData[partsDataLen++] = part.type >> 8;
				fieldDesc |= 1 << 14;
				RESTRICTVERSION(93, 0);
			}

			//Extra Temperature (2nd byte optional, 1st required), 1 to 2 bytes
			//Store temperature as an offset of 21C(294.15K) or go into a 16byte int and store the whole thing
			if(fabs(part.temp-294.15f)<127)
			{
				tempTemp = int(floor(part.temp-294.15f+0.5f));
				partsData[partsDataLen++] = tempTemp;
			}
			else
			{
				fieldDesc |= 1;
				tempTemp = (int)(part.temp+0.5f);
				partsData[partsDataLen++] = tempTemp;
				partsData[partsDataLen++] = tempTemp >> 8;
			}

			if (fieldDesc & (1 << 15))
			{
				fieldDesc3Loc = partsDataLen++;
			}

			//Life (optional), 1 to 2 bytes
			if(part.life)
			{
				int life = part.life;
				if (life > 0xFFFF)
					life = 0xFFFF;
				else if (life < 0)
					life = 0;
				fieldDesc |= 1 << 1;
				partsData[partsDataLen++] = life;
				if (life & 0xFF00)
				{
					fieldDesc |= 1 << 2;
					partsData[partsDataLen++] = life >> 8;
				}
			}

			//Tmp (optional), 1, 2, or 4 bytes
			if(part.tmp)
			{
				fieldDesc |= 1 << 3;
				partsData[partsDataLen++] = part.tmp;
				if(part.tmp & 0xFFFFFF00)
				{
					fieldDesc |= 1 << 4;
					partsData[partsDataLen++] = part.tmp >> 8;
					if(part.tmp & 0xFFFF0000)
					{
						fieldDesc |= 1 << 12;
						partsData[partsDataLen++] = (part.tmp&0xFF000000)>>24;
						partsData[partsDataLen++] = (part.tmp&0x00FF0000)>>16;
					}
				}
			}

			//Ctype (optional), 1 or 4 bytes
			if(part.ctype)
			{
				fieldDesc |= 1 << 5;
				partsData[partsDataLen++] = part.ctype;
				if(part.ctype & 0xFFFFFF00)
				{
					fieldDesc |= 1 << 9;
					partsData[partsDataLen++] = (part.ctype&0xFF000000)>>24;
					partsData[partsDataLen++] = (part.ctype&0x00FF0000)>>16;
					partsData[partsDataLen++] = (part.ctype&0x0000FF00)>>8;
				}
			}

			//Dcolour (optional), 4 bytes
			if(part.dcolour && (part.dcolour & 0xFF000000 || part.type == PT_LIFE))
			{
				fieldDesc |= 1 << 6;
				partsData[partsDataLen++] = (part.dcolour&0xFF000000)>>24;
				partsData[partsDataLen++] = (part.dcolour&0x00FF0000)>>16;
				partsData[partsDataLen++] = (part.dcolour&0x0000FF00)>>8;
				partsData[partsDataLen++] = (part.dcolour&0x000000FF);
			}

			//VX (optional), 1 byte
			if(fabs(part.vx) > 0.001f)
			{
				fieldDesc |= 1 << 7;
				vTemp = (int)(part.vx*16.0f+127.5f);
				if (vTemp<0) vTemp=0;
				if (vTemp>255) vTemp=255;
				partsData[partsDataLen++] = vTemp;
			}

			//VY (optional), 1 byte
			if(fabs(part.vy) > 0.001f)
			{
				fieldDesc |= 1 << 8;
				vTemp = (int)(part.vy*16.0f+127.5f);
				if (vTemp<0) vTemp=0;
				if (vTemp>255) vTemp=255;
				partsData[partsDataLen++] = vTemp;
			}

			//Tmp2 (optional), 1 or 2 bytes
			if(part.tmp2)
			{
				fieldDesc |= 1 << 10;
				partsData[partsDataLen++] = part.tmp2;
				if(part.tmp2 & 0xFF00)
				{
					fieldDesc |= 1 << 11;
					partsData[partsDataLen++] = part.tmp2 >> 8;
				}
			}

			//tmp3 and tmp4, 4 bytes
			if (fieldDesc & (1 << 13))
			{
				partsData[partsDataLen++] = tmp3     ;
				partsData[partsDataLen++] = tmp3 >> 8;
				partsData[partsDataLen++] = tmp4     ;
				partsData[partsDataLen++] = tmp4 >> 8;
				if (fieldDesc & (1 << 16))
				{
					partsData[partsDataLen++] = tmp3 >> 16;
					partsData[partsDataLen++] = tmp3 >> 24;
					partsData[partsDataLen++] = tmp4 >> 16;
					partsData[partsDataLen++] = tmp4 >> 24;
				}
			}

			//Write the field descriptor
			partsData[fieldDescLoc] = fieldDesc;
			partsData[fieldDescLoc+1] = fieldDesc>>8;
			if (fieldDesc & (1 << 15))
			{
				partsData[fieldDesc3Loc] = fieldDesc>>16;
			}

			if (part.type == PT_SOAP)
				soapCount++;

			if (part.type == PT_RPEL && part.ctype)
			{
				RESTRICTVERSION(91, 4);
			}
			else if (part.type == PT_NWHL && part.tmp)
			{
				RESTRICTVERSION(91, 5);
			}
			if (part.type == PT_HEAC || part.type == PT_SAWD || part.type == PT_POLO
					|| part.type == PT_RFRG || part.type == PT_RFGL || part.type == PT_LSNS)
			{
				RESTRICTVERSION(92, 0);
			}
			else if ((part.type == PT_FRAY || part.type == PT_INVIS) && part.tmp)
			{
				RESTRICTVERSION(92, 0);
			}
			else if (part.type == PT_PIPE || part.type == PT_PPIP)
			{
				RESTRICTVERSION(93, 0);
			}
			if (part.type == PT_TSNS || part.type == PT_PSNS
			        || part.type == PT_HSWC || part.type == PT_PUMP)
			{
				if (part.tmp == 1)
				{
					RESTRICTVERSION(93, 0);
				}
			}
			if (PMAPBITS > 8)
			{
				for (auto index : possiblyCarriesType)
				{
					if (builtinElements[part.type].CarriesTypeIn & (1U << index))
					{
						auto *prop = reinterpret_cast<const int *>(reinterpret_cast<const char *>(&part) + properties[index].Offset);
						if (TYP(*prop) > 0xFF)
						{
							RESTRICTVERSION(93, 0);
						}
					}
				}
			}
			if (part.type == PT_LDTC)
			{
				RESTRICTVERSION(94, 0);
			}
			if (part.type == PT_TSNS || part.type == PT_PSNS)
			{
				if (part.tmp == 2)
				{
					RESTRICTVERSION(94, 0);
				}
			}
			if (part.type == PT_LSNS)
			{
				if (part.tmp >= 1 && part.tmp <= 3)
				{
					RESTRICTVERSION(95, 0);
				}
			}
			if (part.type == PT_LIFE)
			{
				RESTRICTVERSION(96, 0);
			}
			if (part.type == PT_GLAS && part.life > 0)
			{
				RESTRICTVERSION(97, 0);
			}
			if (PressureInTmp3(part.type))
			{
				RESTRICTVERSION(97, 0);
			}
			if (part.type == PT_CONV && part.tmp2 != 0)
			{
				RESTRICTVERSION(97, 0);
			}
			if (part.type == PT_RSST || part.type == PT_RSSS)
			{
				RESTRICTVERSION(98, 0);
			}
			if (part.type == PT_ETRD && (part.tmp || part.tmp2))
			{
				RESTRICTVERSION(98, 0);
			}

			//Get the pmap entry for the next particle in the same position
			i = partsPosLink[i];
		}
	}

	std::vector<PaletteItem> paletteData;
	for (int ID : paletteSet)
	{
		paletteData.push_back(GameSave::PaletteItem(elements[ID].Identifier, ID));
	}

	unsigned int soapLinkDataLen = 0;
	std::vector<unsigned char> soapLinkData(3*soapCount);
	if (soapCount)
	{

		//Iterate through particles in the same order that they were saved
		for (auto pos : partS.OriginRect().Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		{
			//Find the first particle in this position
			auto i = partsPosFirstMap[pos];

			//Loop while there is a pmap entry
			while (i)
			{
				//Turn pmap entry into a partsptr index
				i = i>>8;

				if (particles[i].type==PT_SOAP)
				{
					//Only save forward link for each particle, back links can be deduced from other forward links
					//linkedIndex is index within saved particles + 1, 0 means not saved or no link

					unsigned linkedIndex = 0;
					if ((particles[i].ctype&2) && particles[i].tmp>=0 && particles[i].tmp<NPART)
					{
						linkedIndex = partsSaveIndex[particles[i].tmp];
					}
					soapLinkData[soapLinkDataLen++] = (linkedIndex&0xFF0000)>>16;
					soapLinkData[soapLinkDataLen++] = (linkedIndex&0x00FF00)>>8;
					soapLinkData[soapLinkDataLen++] = (linkedIndex&0x0000FF);
				}

				//Get the pmap entry for the next particle in the same position
				i = partsPosLink[i];
			}
		}
	}

	for (size_t i = 0; i < signs.size(); i++)
	{
		if(signs[i].text.length() && partS.OriginRect().Contains({ signs[i].x, signs[i].y }))
		{
			int x, y, w, h;
			bool v95 = false;
			signs[i].getDisplayText(nullptr, x, y, w, h, true, &v95);
			if (v95)
			{
				RESTRICTVERSION(95, 0);
			}
		}
	}

	bson b;
	b.data = nullptr;
	auto bson_deleter = [](bson * b) { bson_destroy(b); };
	// Use unique_ptr with a custom deleter to ensure that bson_destroy is called even when an exception is thrown
	std::unique_ptr<bson, decltype(bson_deleter)> b_ptr(&b, bson_deleter);

	set_bson_err_handler([](const char* err) { throw BuildException("BSON error when parsing save: " + ByteString(err).FromUtf8()); });
	bson_init(&b);
	bson_append_start_object(&b, "origin");
	bson_append_int(&b, "majorVersion", int(effectiveVersion[0]));
	bson_append_int(&b, "minorVersion", int(effectiveVersion[1]));
	bson_append_int(&b, "buildNum", APP_VERSION.build);
	bson_append_int(&b, "modId", MOD_ID);
	bson_append_string(&b, "releaseType", ByteString(1, IDENT_RELTYPE).c_str());
	bson_append_string(&b, "platform", IDENT_PLATFORM);
	bson_append_string(&b, "ident", IDENT);
	bson_append_finish_object(&b);
	if (gravityMode == GRAV_CUSTOM)
	{
		bson_append_double(&b, "customGravityX", double(customGravityX));
		bson_append_double(&b, "customGravityY", double(customGravityY));
		RESTRICTVERSION(97, 0);
	}
	bson_append_start_object(&b, "minimumVersion");
	bson_append_int(&b, "major", int(minimumVersion[0]));
	bson_append_int(&b, "minor", int(minimumVersion[1]));
	bson_append_finish_object(&b);


	bson_append_bool(&b, "waterEEnabled", waterEEnabled);
	bson_append_bool(&b, "legacyEnable", legacyEnable);
	bson_append_bool(&b, "gravityEnable", gravityEnable);
	bson_append_bool(&b, "aheat_enable", aheatEnable);
	bson_append_bool(&b, "paused", paused);
	bson_append_int(&b, "gravityMode", gravityMode);
	bson_append_int(&b, "airMode", airMode);
	if (fabsf(ambientAirTemp - (R_TEMP + 273.15f)) > 0.0001f)
	{
		bson_append_double(&b, "ambientAirTemp", double(ambientAirTemp));
		RESTRICTVERSION(96, 0);
	}
	bson_append_int(&b, "edgeMode", edgeMode);

	if (stkm.hasData())
	{
		bson_append_start_object(&b, "stkm");
		if (stkm.rocketBoots1)
			bson_append_bool(&b, "rocketBoots1", stkm.rocketBoots1);
		if (stkm.rocketBoots2)
			bson_append_bool(&b, "rocketBoots2", stkm.rocketBoots2);
		if (stkm.fan1)
			bson_append_bool(&b, "fan1", stkm.fan1);
		if (stkm.fan2)
			bson_append_bool(&b, "fan2", stkm.fan2);
		if (stkm.rocketBootsFigh.size())
		{
			bson_append_start_array(&b, "rocketBootsFigh");
			for (unsigned int fighNum : stkm.rocketBootsFigh)
				bson_append_int(&b, "num", fighNum);
			bson_append_finish_array(&b);
		}
		if (stkm.fanFigh.size())
		{
			bson_append_start_array(&b, "fanFigh");
			for (unsigned int fighNum : stkm.fanFigh)
				bson_append_int(&b, "num", fighNum);
			bson_append_finish_array(&b);
		}
		bson_append_finish_object(&b);
	}

	bson_append_int(&b, "pmapbits", pmapbits);
	if (partsDataLen)
	{
		bson_append_binary(&b, "parts", (char)BSON_BIN_USER, reinterpret_cast<const char *>(partsData.data()), partsDataLen);

		if (paletteData.size())
		{
			bson_append_start_array(&b, "palette");
			for(auto iter = paletteData.begin(), end = paletteData.end(); iter != end; ++iter)
			{
				bson_append_int(&b, (*iter).first.c_str(), (*iter).second);
			}
			bson_append_finish_array(&b);
		}

		if (partsPosDataLen)
			bson_append_binary(&b, "partsPos", (char)BSON_BIN_USER, reinterpret_cast<const char *>(partsPosData.data()), partsPosDataLen);
	}
	if (hasWallData)
		bson_append_binary(&b, "wallMap", (char)BSON_BIN_USER, (const char *)wallData.data(), wallDataLen);
	if (fanDataLen)
		bson_append_binary(&b, "fanMap", (char)BSON_BIN_USER, reinterpret_cast<const char *>(fanData.data()), fanDataLen);
	if (hasPressure && pressDataLen)
		bson_append_binary(&b, "pressMap", (char)BSON_BIN_USER, reinterpret_cast<const char *>(pressData.data()), pressDataLen);
	if (hasPressure && vxDataLen)
		bson_append_binary(&b, "vxMap", (char)BSON_BIN_USER, reinterpret_cast<const char *>(vxData.data()), vxDataLen);
	if (hasPressure && vyDataLen)
		bson_append_binary(&b, "vyMap", (char)BSON_BIN_USER, reinterpret_cast<const char *>(vyData.data()), vyDataLen);
	if (hasAmbientHeat && this->aheatEnable && ambientDataLen)
		bson_append_binary(&b, "ambientMap", (char)BSON_BIN_USER, reinterpret_cast<const char *>(ambientData.data()), ambientDataLen);
	if (soapLinkDataLen)
		bson_append_binary(&b, "soapLinks", (char)BSON_BIN_USER, reinterpret_cast<const char *>(soapLinkData.data()), soapLinkDataLen);
	bson_append_binary(&b, "blockAir", (char)BSON_BIN_USER, reinterpret_cast<const char *>(blockAirData.data()), blockAirData.size());
	if (ensureDeterminism)
	{
		bson_append_bool(&b, "ensureDeterminism", ensureDeterminism);
		bson_append_long(&b, "frameCount", int64_t(frameCount));
		bson_append_binary(&b, "rngState", (char)BSON_BIN_USER, reinterpret_cast<const char *>(&rngState), sizeof(rngState));
		RESTRICTVERSION(98, 0);
	}
	if (gravityEnable)
	{
		bson_append_binary(&b, "gravity", (char)BSON_BIN_USER, reinterpret_cast<const char *>(gravityData.data()), gravityData.size());
	}
	unsigned int signsCount = 0;
	for (size_t i = 0; i < signs.size(); i++)
	{
		if(signs[i].text.length() && partS.OriginRect().Contains({ signs[i].x, signs[i].y }))
		{
			signsCount++;
		}
	}
	if (signsCount)
	{
		bson_append_start_array(&b, "signs");
		for (size_t i = 0; i < signs.size(); i++)
		{
			if(signs[i].text.length() && partS.OriginRect().Contains({ signs[i].x, signs[i].y }))
			{
				bson_append_start_object(&b, "sign");
				bson_append_string(&b, "text", signs[i].text.ToUtf8().c_str());
				bson_append_int(&b, "justification", signs[i].ju);
				bson_append_int(&b, "x", signs[i].x);
				bson_append_int(&b, "y", signs[i].y);
				bson_append_finish_object(&b);
			}
		}
		bson_append_finish_array(&b);
	}
	if (authors.size())
	{
		bson_append_start_object(&b, "authors");
		ConvertJsonToBson(&b, authors);
		bson_append_finish_object(&b);
	}
	if (bson_finish(&b) == BSON_ERROR)
		throw BuildException("Error building bson data");

	unsigned char *finalData = (unsigned char*)bson_data(&b);
	unsigned int finalDataLen = bson_size(&b);


	std::vector<char> outputData;
	switch (auto status = BZ2WCompress(outputData, std::span(reinterpret_cast<const char *>(finalData), finalDataLen)))
	{
	case BZ2WCompressOk: break;
	case BZ2WCompressNomem: throw BuildException(String::Build("Save error, out of memory"));
	default: throw BuildException(String::Build("Cannot compress: status ", int(status)));
	}
	auto compressedSize = int(outputData.size());

	if constexpr (DEBUG)
	{
		printf("compressed data: %d\n", compressedSize);
	}
	outputData.resize(compressedSize + 12);

	auto header = (unsigned char *)&outputData[compressedSize];
	header[0] = 'O';
	header[1] = 'P';
	header[2] = 'S';
	header[3] = '1';
	header[4] = effectiveVersion[0];
	header[5] = CELL;
	header[6] = blockS.X;
	header[7] = blockS.Y;
	header[8] = finalDataLen;
	header[9] = finalDataLen >> 8;
	header[10] = finalDataLen >> 16;
	header[11] = finalDataLen >> 24;

	// move header to front
	std::rotate(outputData.begin(), outputData.begin() + compressedSize, outputData.end());

	// Mark save as incompatible with latest release
	bool fakeFromNewerVersion = ALLOW_FAKE_NEWER_VERSION && currentVersion < minimumVersion;
	return { fakeFromNewerVersion, outputData };
}

static void ConvertBsonToJson(bson_iterator *iter, Json::Value *j, int depth)
{
	bson_iterator subiter;
	bson_iterator_subiterator(iter, &subiter);
	while (bson_iterator_next(&subiter))
	{
		ByteString key = bson_iterator_key(&subiter);
		if (bson_iterator_type(&subiter) == BSON_STRING)
			(*j)[key] = bson_iterator_string(&subiter);
		else if (bson_iterator_type(&subiter) == BSON_BOOL)
			(*j)[key] = bson_iterator_bool(&subiter);
		else if (bson_iterator_type(&subiter) == BSON_INT)
			(*j)[key] = bson_iterator_int(&subiter);
		else if (bson_iterator_type(&subiter) == BSON_LONG)
			(*j)[key] = (Json::Value::UInt64)bson_iterator_long(&subiter);
		else if (bson_iterator_type(&subiter) == BSON_ARRAY && depth < 5)
		{
			bson_iterator arrayiter;
			bson_iterator_subiterator(&subiter, &arrayiter);
			int length = 0, length2 = 0;
			while (bson_iterator_next(&arrayiter))
			{
				if (bson_iterator_type(&arrayiter) == BSON_OBJECT && !strcmp(bson_iterator_key(&arrayiter), "part"))
				{
					Json::Value tempPart;
					ConvertBsonToJson(&arrayiter, &tempPart, depth + 1);
					(*j)["links"].append(tempPart);
					length++;
				}
				else if (bson_iterator_type(&arrayiter) == BSON_INT && !strcmp(bson_iterator_key(&arrayiter), "saveID"))
				{
					(*j)["links"].append(bson_iterator_int(&arrayiter));
				}
				length2++;
				if (length > (int)(40 / ((depth+1) * (depth+1))) || length2 > 50)
					break;
			}
		}
	}
}

std::set<int> GetNestedSaveIDs(Json::Value j)
{
	Json::Value::Members members = j.getMemberNames();
	std::set<int> saveIDs = std::set<int>();
	for (Json::Value::Members::iterator iter = members.begin(), end = members.end(); iter != end; ++iter)
	{
		ByteString member = *iter;
		if (member == "id" && j[member].isInt())
			saveIDs.insert(j[member].asInt());
		else if (j[member].isArray())
		{
			for (Json::Value::ArrayIndex i = 0; i < j[member].size(); i++)
			{
				// only supports objects and ints here because that is all we need
				if (j[member][i].isInt())
				{
					saveIDs.insert(j[member][i].asInt());
					continue;
				}
				if (!j[member][i].isObject())
					continue;
				std::set<int> nestedSaveIDs = GetNestedSaveIDs(j[member][i]);
				saveIDs.insert(nestedSaveIDs.begin(), nestedSaveIDs.end());
			}
		}
	}
	return saveIDs;
}

// converts a json object to bson
static void ConvertJsonToBson(bson *b, Json::Value j, int depth)
{
	Json::Value::Members members = j.getMemberNames();
	for (Json::Value::Members::iterator iter = members.begin(), end = members.end(); iter != end; ++iter)
	{
		ByteString member = *iter;
		if (j[member].isString())
			bson_append_string(b, member.c_str(), j[member].asCString());
		else if (j[member].isBool())
			bson_append_bool(b, member.c_str(), j[member].asBool());
		else if (j[member].type() == Json::intValue)
			bson_append_int(b, member.c_str(), j[member].asInt());
		else if (j[member].type() == Json::uintValue)
			bson_append_long(b, member.c_str(), j[member].asInt64());
		else if (j[member].isArray())
		{
			bson_append_start_array(b, member.c_str());
			std::set<int> saveIDs = std::set<int>();
			int length = 0;
			for (Json::Value::ArrayIndex i = 0; i < j[member].size(); i++)
			{
				// only supports objects and ints here because that is all we need
				if (j[member][i].isInt())
				{
					saveIDs.insert(j[member][i].asInt());
					continue;
				}
				if (!j[member][i].isObject())
					continue;
				if (depth > 4 || length > (int)(40 / ((depth+1) * (depth+1))))
				{
					std::set<int> nestedSaveIDs = GetNestedSaveIDs(j[member][i]);
					saveIDs.insert(nestedSaveIDs.begin(), nestedSaveIDs.end());
				}
				else
				{
					bson_append_start_object(b, "part");
					ConvertJsonToBson(b, j[member][i], depth+1);
					bson_append_finish_object(b);
				}
				length++;
			}
			for (std::set<int>::iterator iter = saveIDs.begin(), end = saveIDs.end(); iter != end; ++iter)
			{
				bson_append_int(b, "saveID", *iter);
			}
			bson_append_finish_array(b);
		}
	}
}

bool GameSave::PressureInTmp3(int type)
{
	return type == PT_QRTZ || type == PT_GLAS || type == PT_TUNG;
}

GameSave& GameSave::operator << (Particle &v)
{
	if(particlesCount<NPART && v.type)
	{
		particles[particlesCount++] = v;
	}
	return *this;
}

GameSave& GameSave::operator << (sign &v)
{
	if(signs.size()<MAXSIGNS && v.text.length())
		signs.push_back(v);
	return *this;
}
