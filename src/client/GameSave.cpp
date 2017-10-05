#include "common/tpt-minmax.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <set>
#include <bzlib.h>
#include "Config.h"
#include "Format.h"
#include "GameSave.h"
#include "simulation/SimulationData.h"
#include "ElementClasses.h"
extern "C"
{
	#include "hmap.h"
}

GameSave::GameSave(GameSave & save) :
waterEEnabled(save.waterEEnabled),
legacyEnable(save.legacyEnable),
gravityEnable(save.gravityEnable),
aheatEnable(save.aheatEnable),
paused(save.paused),
gravityMode(save.gravityMode),
airMode(save.airMode),
edgeMode(save.edgeMode),
signs(save.signs),
palette(save.palette),
expanded(save.expanded),
hasOriginalData(save.hasOriginalData),
originalData(save.originalData)
{
	InitData();
	if (save.expanded)
	{
		setSize(save.blockWidth, save.blockHeight);

		std::copy(save.particles, save.particles+NPART, particles);
		for (int j = 0; j < blockHeight; j++)
		{
			std::copy(save.blockMap[j], save.blockMap[j]+blockWidth, blockMap[j]);
			std::copy(save.fanVelX[j], save.fanVelX[j]+blockWidth, fanVelX[j]);
			std::copy(save.fanVelY[j], save.fanVelY[j]+blockWidth, fanVelY[j]);
			std::copy(save.pressure[j], save.pressure[j]+blockWidth, pressure[j]);
			std::copy(save.velocityX[j], save.velocityX[j]+blockWidth, velocityX[j]);
			std::copy(save.velocityY[j], save.velocityY[j]+blockWidth, velocityY[j]);
			std::copy(save.ambientHeat[j], save.ambientHeat[j]+blockWidth, ambientHeat[j]);
		}
	}
	else
	{
		blockWidth = save.blockWidth;
		blockHeight = save.blockHeight;
	}
	particlesCount = save.particlesCount;
	authors = save.authors;
}

GameSave::GameSave(int width, int height)
{
	InitData();
	InitVars();
	hasOriginalData = false;
	expanded = true;
	setSize(width, height);
}

GameSave::GameSave(std::vector<char> data)
{
	blockWidth = 0;
	blockHeight = 0;

	InitData();
	InitVars();
	expanded = false;
	hasOriginalData = true;
	originalData = data;
	try
	{
		Expand();
	}
	catch(ParseException & e)
	{
		std::cout << e.what() << std::endl;
		dealloc();	//Free any allocated memory
		throw;
	}
	Collapse();
}

GameSave::GameSave(std::vector<unsigned char> data)
{
	blockWidth = 0;
	blockHeight = 0;

	InitData();
	InitVars();
	expanded = false;
	hasOriginalData = true;
	originalData = std::vector<char>(data.begin(), data.end());
	try
	{
		Expand();
	}
	catch(ParseException & e)
	{
		std::cout << e.what() << std::endl;
		dealloc();	//Free any allocated memory
		throw;
	}
	Collapse();
}

GameSave::GameSave(char * data, int dataSize)
{
	blockWidth = 0;
	blockHeight = 0;

	InitData();
	InitVars();
	expanded = false;
	hasOriginalData = true;
	originalData = std::vector<char>(data, data+dataSize);
#ifdef DEBUG
	std::cout << "Creating Expanded save from data" << std::endl;
#endif
	try
	{
		Expand();
	}
	catch(ParseException & e)
	{
		std::cout << e.what() << std::endl;
		dealloc();	//Free any allocated memory
		throw;
	}
	//Collapse();
}

void GameSave::InitData()
{
	blockMap = NULL;
	fanVelX = NULL;
	fanVelY = NULL;
	particles = NULL;
	pressure = NULL;
	velocityX = NULL;
	velocityY = NULL;
	ambientHeat = NULL;
	fromNewerVersion = false;
	hasAmbientHeat = false;
	authors.clear();
}

void GameSave::InitVars()
{
	waterEEnabled = false;
	legacyEnable = false;
	gravityEnable = false;
	aheatEnable = false;
	paused = false;
	gravityMode = 0;
	airMode = 0;
	edgeMode = 0;
	translated.x = translated.y = 0;
}

bool GameSave::Collapsed()
{
	return !expanded;
}

void GameSave::Expand()
{
	if(hasOriginalData && !expanded)
	{
		InitVars();
		expanded = true;
		read(&originalData[0], originalData.size());
	}
}

void GameSave::Collapse()
{
	if(expanded && hasOriginalData)
	{
		expanded = false;
		dealloc();
		signs.clear();
	}
}

void GameSave::read(char * data, int dataSize)
{
	if(dataSize > 15)
	{
		if ((data[0]==0x66 && data[1]==0x75 && data[2]==0x43) || (data[0]==0x50 && data[1]==0x53 && data[2]==0x76))
		{
			readPSv(data, dataSize);
		}
		else if(data[0] == 'O' && data[1] == 'P' && data[2] == 'S')
		{
			if (data[3] != '1')
				throw ParseException(ParseException::WrongVersion, "Save format from newer version");
			readOPS(data, dataSize);
		}
		else
		{
			std::cerr << "Got Magic number '" << data[0] << data[1] << data[2] << "'" << std::endl;
			throw ParseException(ParseException::Corrupt, "Invalid save format");
		}
	}
	else
	{
		throw ParseException(ParseException::Corrupt, "No data");
	}
}

template <typename T>
T ** GameSave::Allocate2DArray(int blockWidth, int blockHeight, T defaultVal)
{
	T ** temp = new T*[blockHeight];
	for (int y = 0; y < blockHeight; y++)
	{
		temp[y] = new T[blockWidth];
		std::fill(&temp[y][0], &temp[y][0]+blockWidth, defaultVal);
	}
	return temp;
}

void GameSave::setSize(int newWidth, int newHeight)
{
	this->blockWidth = newWidth;
	this->blockHeight = newHeight;

	particlesCount = 0;
	particles = new Particle[NPART];

	blockMap = Allocate2DArray<unsigned char>(blockWidth, blockHeight, 0);
	fanVelX = Allocate2DArray<float>(blockWidth, blockHeight, 0.0f);
	fanVelY = Allocate2DArray<float>(blockWidth, blockHeight, 0.0f);
	pressure = Allocate2DArray<float>(blockWidth, blockHeight, 0.0f);
	velocityX = Allocate2DArray<float>(blockWidth, blockHeight, 0.0f);
	velocityY = Allocate2DArray<float>(blockWidth, blockHeight, 0.0f);
	ambientHeat = Allocate2DArray<float>(blockWidth, blockHeight, 0.0f);
}

std::vector<char> GameSave::Serialise()
{
	unsigned int dataSize;
	char * data = Serialise(dataSize);
	if (data == NULL)
		return std::vector<char>();
	std::vector<char> dataVect(data, data+dataSize);
	delete[] data;
	return dataVect;
}

char * GameSave::Serialise(unsigned int & dataSize)
{
	return serialiseOPS(dataSize);
}

vector2d GameSave::Translate(vector2d translate)
{
	if (Collapsed())
		Expand();
	int nx, ny;
	vector2d pos;
	vector2d translateReal = translate;
	float minx = 0, miny = 0, maxx = 0, maxy = 0;
	// determine minimum and maximum position of all particles / signs
	for (size_t i = 0; i < signs.size(); i++)
	{
		pos = v2d_new(signs[i].x, signs[i].y);
		pos = v2d_add(pos,translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx < minx)
			minx = nx;
		if (ny < miny)
			miny = ny;
		if (nx > maxx)
			maxx = nx;
		if (ny > maxy)
			maxy = ny;
	}
	for (int i = 0; i < particlesCount; i++)
	{
		if (!particles[i].type) continue;
		pos = v2d_new(particles[i].x, particles[i].y);
		pos = v2d_add(pos,translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx < minx)
			minx = nx;
		if (ny < miny)
			miny = ny;
		if (nx > maxx)
			maxx = nx;
		if (ny > maxy)
			maxy = ny;
	}
	// determine whether corrections are needed. If moving in this direction would delete stuff, expand the save
	vector2d backCorrection = v2d_new(
		(minx < 0) ? (-floor(minx / CELL)) : 0,
		(miny < 0) ? (-floor(miny / CELL)) : 0
	);
	int blockBoundsX = int(maxx / CELL) + 1, blockBoundsY = int(maxy / CELL) + 1;
	vector2d frontCorrection = v2d_new(
		(blockBoundsX > blockWidth) ? (blockBoundsX - blockWidth) : 0,
		(blockBoundsY > blockHeight) ? (blockBoundsY - blockHeight) : 0
	);

	// get new width based on corrections
	int newWidth = (blockWidth + backCorrection.x + frontCorrection.x) * CELL;
	int newHeight = (blockHeight + backCorrection.y + frontCorrection.y) * CELL;
	if (newWidth > XRES)
		frontCorrection.x = backCorrection.x = 0;
	if (newHeight > YRES)
		frontCorrection.y = backCorrection.y = 0;

	// call Transform to do the transformation we wanted when calling this function
	translate = v2d_add(translate, v2d_multiply_float(backCorrection, CELL));
	Transform(m2d_identity, translate, translateReal,
	    (blockWidth + backCorrection.x + frontCorrection.x) * CELL,
	    (blockHeight + backCorrection.y + frontCorrection.y) * CELL
	);

	// return how much we corrected. This is used to offset the position of the current stamp
	// otherwise it would attempt to recenter it with the current size
	return v2d_add(v2d_multiply_float(backCorrection, -CELL), v2d_multiply_float(frontCorrection, CELL));
}

void GameSave::Transform(matrix2d transform, vector2d translate)
{
	if (Collapsed())
		Expand();

	int width = blockWidth*CELL, height = blockHeight*CELL, newWidth, newHeight;
	vector2d tmp, ctl, cbr;
	vector2d cornerso[4];
	vector2d translateReal = translate;
	// undo any translation caused by rotation
	cornerso[0] = v2d_new(0,0);
	cornerso[1] = v2d_new(width-1,0);
	cornerso[2] = v2d_new(0,height-1);
	cornerso[3] = v2d_new(width-1,height-1);
	for (int i = 0; i < 4; i++)
	{
		tmp = m2d_multiply_v2d(transform,cornerso[i]);
		if (i==0) ctl = cbr = tmp; // top left, bottom right corner
		if (tmp.x<ctl.x) ctl.x = tmp.x;
		if (tmp.y<ctl.y) ctl.y = tmp.y;
		if (tmp.x>cbr.x) cbr.x = tmp.x;
		if (tmp.y>cbr.y) cbr.y = tmp.y;
	}
	// casting as int doesn't quite do what we want with negative numbers, so use floor()
	tmp = v2d_new(floor(ctl.x+0.5f),floor(ctl.y+0.5f));
	translate = v2d_sub(translate,tmp);
	newWidth = floor(cbr.x+0.5f)-floor(ctl.x+0.5f)+1;
	newHeight = floor(cbr.y+0.5f)-floor(ctl.y+0.5f)+1;
	Transform(transform, translate, translateReal, newWidth, newHeight);
}

// transform is a matrix describing how we want to rotate this save
// translate can vary depending on whether the save is bring rotated, or if a normal translate caused it to expand
// translateReal is the original amount we tried to translate, used to calculate wall shifting
void GameSave::Transform(matrix2d transform, vector2d translate, vector2d translateReal, int newWidth, int newHeight)
{
	if (Collapsed())
		Expand();

	if (newWidth>XRES) newWidth = XRES;
	if (newHeight>YRES) newHeight = YRES;

	int x, y, nx, ny, newBlockWidth = newWidth / CELL, newBlockHeight = newHeight / CELL;
	vector2d pos, vel;

	unsigned char ** blockMapNew;
	float **fanVelXNew, **fanVelYNew, **pressureNew, **velocityXNew, **velocityYNew, **ambientHeatNew;

	blockMapNew = Allocate2DArray<unsigned char>(newBlockWidth, newBlockHeight, 0);
	fanVelXNew = Allocate2DArray<float>(newBlockWidth, newBlockHeight, 0.0f);
	fanVelYNew = Allocate2DArray<float>(newBlockWidth, newBlockHeight, 0.0f);
	pressureNew = Allocate2DArray<float>(newBlockWidth, newBlockHeight, 0.0f);
	velocityXNew = Allocate2DArray<float>(newBlockWidth, newBlockHeight, 0.0f);
	velocityYNew = Allocate2DArray<float>(newBlockWidth, newBlockHeight, 0.0f);
	ambientHeatNew = Allocate2DArray<float>(newBlockWidth, newBlockHeight, 0.0f);

	// rotate and translate signs, parts, walls
	for (size_t i = 0; i < signs.size(); i++)
	{
		pos = v2d_new(signs[i].x, signs[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx<0 || nx>=newWidth || ny<0 || ny>=newHeight)
		{
			signs[i].text[0] = 0;
			continue;
		}
		signs[i].x = nx;
		signs[i].y = ny;
	}
	for (int i = 0; i < particlesCount; i++)
	{
		if (!particles[i].type) continue;
		pos = v2d_new(particles[i].x, particles[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx<0 || nx>=newWidth || ny<0 || ny>=newHeight)
		{
			particles[i].type = PT_NONE;
			continue;
		}
		particles[i].x = nx;
		particles[i].y = ny;
		vel = v2d_new(particles[i].vx, particles[i].vy);
		vel = m2d_multiply_v2d(transform, vel);
		particles[i].vx = vel.x;
		particles[i].vy = vel.y;
	}

	// translate walls and other grid items when the stamp is shifted more than 4 pixels in any direction
	int translateX = 0, translateY = 0;
	if (translateReal.x > 0 && ((int)translated.x%CELL == 3
	                        || (translated.x < 0 && (int)translated.x%CELL == 0)))
		translateX = CELL;
	else if (translateReal.x < 0 && ((int)translated.x%CELL == -3
	                             || (translated.x > 0 && (int)translated.x%CELL == 0)))
		translateX = -CELL;
	if (translateReal.y > 0 && ((int)translated.y%CELL == 3
	                        || (translated.y < 0 && (int)translated.y%CELL == 0)))
		translateY = CELL;
	else if (translateReal.y < 0 && ((int)translated.y%CELL == -3
	                             || (translated.y > 0 && (int)translated.y%CELL == 0)))
		translateY = -CELL;

	for (y=0; y<blockHeight; y++)
		for (x=0; x<blockWidth; x++)
		{
			pos = v2d_new(x*CELL+CELL*0.4f+translateX, y*CELL+CELL*0.4f+translateY);
			pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
			nx = pos.x/CELL;
			ny = pos.y/CELL;
			if (pos.x<0 || nx>=newBlockWidth || pos.y<0 || ny>=newBlockHeight)
				continue;
			if (blockMap[y][x])
			{
				blockMapNew[ny][nx] = blockMap[y][x];
				if (blockMap[y][x]==WL_FAN)
				{
					vel = v2d_new(fanVelX[y][x], fanVelY[y][x]);
					vel = m2d_multiply_v2d(transform, vel);
					fanVelXNew[ny][nx] = vel.x;
					fanVelYNew[ny][nx] = vel.y;
				}
			}
			pressureNew[ny][nx] = pressure[y][x];
			velocityXNew[ny][nx] = velocityX[y][x];
			velocityYNew[ny][nx] = velocityY[y][x];
			ambientHeatNew[ny][nx] = ambientHeat[y][x];
		}
	translated = v2d_add(m2d_multiply_v2d(transform, translated), translateReal);

	for (int j = 0; j < blockHeight; j++)
	{
		delete[] blockMap[j];
		delete[] fanVelX[j];
		delete[] fanVelY[j];
		delete[] pressure[j];
		delete[] velocityX[j];
		delete[] velocityY[j];
		delete[] ambientHeat[j];
	}

	blockWidth = newBlockWidth;
	blockHeight = newBlockHeight;

	delete[] blockMap;
	delete[] fanVelX;
	delete[] fanVelY;
	delete[] pressure;
	delete[] velocityX;
	delete[] velocityY;
	delete[] ambientHeat;

	blockMap = blockMapNew;
	fanVelX = fanVelXNew;
	fanVelY = fanVelYNew;
	pressure = pressureNew;
	velocityX = velocityXNew;
	velocityY = velocityYNew;
	ambientHeat = ambientHeatNew;
}

void bson_error_handler(const char *err)
{
	throw ParseException(ParseException::Corrupt, "BSON error when parsing save");
}

void GameSave::CheckBsonFieldUser(bson_iterator iter, const char *field, unsigned char **data, unsigned int *fieldLen)
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

void GameSave::CheckBsonFieldBool(bson_iterator iter, const char *field, bool *flag)
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

void GameSave::CheckBsonFieldInt(bson_iterator iter, const char *field, int *setting)
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

void GameSave::readOPS(char * data, int dataLength)
{
	unsigned char *inputData = (unsigned char*)data, *bsonData = NULL, *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL, *soapLinkData = NULL;
	unsigned char *pressData = NULL, *vxData = NULL, *vyData = NULL, *ambientData = NULL;
	unsigned int inputDataLen = dataLength, bsonDataLen = 0, partsDataLen, partsPosDataLen, fanDataLen, wallDataLen, soapLinkDataLen;
	unsigned int pressDataLen, vxDataLen, vyDataLen, ambientDataLen;
	unsigned partsCount = 0, *partsSimIndex = NULL;
	int *freeIndices = NULL;
	unsigned int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	int savedVersion = inputData[4];
	bson b;
	bson_iterator iter;

	//Block sizes
	blockX = 0;
	blockY = 0;
	blockW = inputData[6];
	blockH = inputData[7];

	//Full size, normalised
	fullX = blockX*CELL;
	fullY = blockY*CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;

	//From newer version
	if (savedVersion > SAVE_VERSION)
	{
		fromNewerVersion = true;
		//throw ParseException(ParseException::WrongVersion, "Save from newer version");
	}

	//Incompatible cell size
	if (inputData[5] > CELL)
		throw ParseException(ParseException::InvalidDimensions, "Incorrect CELL size");

	//Too large/off screen
	if (blockX+blockW > XRES/CELL || blockY+blockH > YRES/CELL)
		throw ParseException(ParseException::InvalidDimensions, "Save too large");

	setSize(blockW, blockH);

	bsonDataLen = ((unsigned)inputData[8]);
	bsonDataLen |= ((unsigned)inputData[9]) << 8;
	bsonDataLen |= ((unsigned)inputData[10]) << 16;
	bsonDataLen |= ((unsigned)inputData[11]) << 24;
	
	//Check for overflows, don't load saves larger than 200MB
	unsigned int toAlloc = bsonDataLen+1;
	if (toAlloc > 209715200 || !toAlloc)
		throw ParseException(ParseException::InvalidDimensions, "Save data too large, refusing");
		
	bsonData = (unsigned char*)malloc(toAlloc);
	if (!bsonData)
		throw ParseException(ParseException::InternalError, "Unable to allocate memory");

	//Make sure bsonData is null terminated, since all string functions need null terminated strings
	//(bson_iterator_key returns a pointer into bsonData, which is then used with strcmp)
	bsonData[bsonDataLen] = 0;

	if (BZ2_bzBuffToBuffDecompress((char*)bsonData, &bsonDataLen, (char*)(inputData+12), inputDataLen-12, 0, 0))
		throw ParseException(ParseException::Corrupt, "Unable to decompress");

	set_bson_err_handler(bson_error_handler);
	bson_init_data_size(&b, (char*)bsonData, bsonDataLen);
	bson_iterator_init(&iter, &b);

	std::vector<sign> tempSigns;

	while (bson_iterator_next(&iter))
	{
		CheckBsonFieldUser(iter, "parts", &partsData, &partsDataLen);
		CheckBsonFieldUser(iter, "partsPos", &partsPosData, &partsPosDataLen);
		CheckBsonFieldUser(iter, "wallMap", &wallData, &wallDataLen);
		CheckBsonFieldUser(iter, "pressMap", &pressData, &pressDataLen);
		CheckBsonFieldUser(iter, "vxMap", &vxData, &vxDataLen);
		CheckBsonFieldUser(iter, "vyMap", &vyData, &vyDataLen);
		CheckBsonFieldUser(iter, "ambientMap", &ambientData, &ambientDataLen);
		CheckBsonFieldUser(iter, "fanMap", &fanData, &fanDataLen);
		CheckBsonFieldUser(iter, "soapLinks", &soapLinkData, &soapLinkDataLen);
		CheckBsonFieldBool(iter, "legacyEnable", &legacyEnable);
		CheckBsonFieldBool(iter, "gravityEnable", &gravityEnable);
		CheckBsonFieldBool(iter, "aheat_enable", &aheatEnable);
		CheckBsonFieldBool(iter, "waterEEnabled", &waterEEnabled);
		CheckBsonFieldBool(iter, "paused", &paused);
		CheckBsonFieldInt(iter, "gravityMode", &gravityMode);
		CheckBsonFieldInt(iter, "airMode", &airMode);
		CheckBsonFieldInt(iter, "edgeMode", &edgeMode);
		if (!strcmp(bson_iterator_key(&iter), "signs"))
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
									tempSign.text = format::CleanString(bson_iterator_string(&signiter), true, true, true).substr(0, 45);
								}
								else if (!strcmp(bson_iterator_key(&signiter), "justification") && bson_iterator_type(&signiter) == BSON_INT)
								{
									tempSign.ju = (sign::Justification)bson_iterator_int(&signiter);
								}
								else if (!strcmp(bson_iterator_key(&signiter), "x") && bson_iterator_type(&signiter) == BSON_INT)
								{
									tempSign.x = bson_iterator_int(&signiter)+fullX;
								}
								else if (!strcmp(bson_iterator_key(&signiter), "y") && bson_iterator_type(&signiter) == BSON_INT)
								{
									tempSign.y = bson_iterator_int(&signiter)+fullY;
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
						std::string id = std::string(bson_iterator_key(&subiter));
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
						else
							fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
					}
				}
#ifdef SNAPSHOT
				if (major > FUTURE_SAVE_VERSION || (major == FUTURE_SAVE_VERSION && minor > FUTURE_MINOR_VERSION))
#else
				if (major > SAVE_VERSION || (major == SAVE_VERSION && minor > MINOR_VERSION))
#endif
				{
					std::stringstream errorMessage;
					errorMessage << "Save from a newer version: Requires version " << major << "." << minor;
					throw ParseException(ParseException::WrongVersion, errorMessage.str());
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
#ifndef RENDERER
		else if (!strcmp(bson_iterator_key(&iter), "authors"))
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
#endif
	}

	//Read wall and fan data
	if(wallData)
	{
		unsigned int j = 0;
		if (blockW * blockH > wallDataLen)
		{
			fprintf(stderr, "Not enough wall data\n");
			goto fail;
		}
		for (unsigned int x = 0; x < blockW; x++)
		{
			for (unsigned int y = 0; y < blockH; y++)
			{
				if (wallData[y*blockW+x])
					blockMap[blockY+y][blockX+x] = wallData[y*blockW+x];

				if (blockMap[y][x]==O_WL_WALLELEC)
					blockMap[y][x]=WL_WALLELEC;
				if (blockMap[y][x]==O_WL_EWALL)
					blockMap[y][x]=WL_EWALL;
				if (blockMap[y][x]==O_WL_DETECT)
					blockMap[y][x]=WL_DETECT;
				if (blockMap[y][x]==O_WL_STREAM)
					blockMap[y][x]=WL_STREAM;
				if (blockMap[y][x]==O_WL_FAN||blockMap[y][x]==O_WL_FANHELPER)
					blockMap[y][x]=WL_FAN;
				if (blockMap[y][x]==O_WL_ALLOWLIQUID)
					blockMap[y][x]=WL_ALLOWLIQUID;
				if (blockMap[y][x]==O_WL_DESTROYALL)
					blockMap[y][x]=WL_DESTROYALL;
				if (blockMap[y][x]==O_WL_ERASE)
					blockMap[y][x]=WL_ERASE;
				if (blockMap[y][x]==O_WL_WALL)
					blockMap[y][x]=WL_WALL;
				if (blockMap[y][x]==O_WL_ALLOWAIR)
					blockMap[y][x]=WL_ALLOWAIR;
				if (blockMap[y][x]==O_WL_ALLOWSOLID)
					blockMap[y][x]=WL_ALLOWPOWDER;
				if (blockMap[y][x]==O_WL_ALLOWALLELEC)
					blockMap[y][x]=WL_ALLOWALLELEC;
				if (blockMap[y][x]==O_WL_EHOLE)
					blockMap[y][x]=WL_EHOLE;
				if (blockMap[y][x]==O_WL_ALLOWGAS)
					blockMap[y][x]=WL_ALLOWGAS;
				if (blockMap[y][x]==O_WL_GRAV)
					blockMap[y][x]=WL_GRAV;
				if (blockMap[y][x]==O_WL_ALLOWENERGY)
					blockMap[y][x]=WL_ALLOWENERGY;

				if (blockMap[y][x] == WL_FAN && fanData)
				{
					if(j+1 >= fanDataLen)
					{
						fprintf(stderr, "Not enough fan data\n");
					}
					fanVelX[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
					fanVelY[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
				}

				if (blockMap[y][x] >= UI_WALLCOUNT)
					blockMap[y][x] = 0;
			}
		}
	}

	//Read pressure data
	if (pressData)
	{
		unsigned int j = 0;
		unsigned char i, i2;
		if (blockW * blockH > pressDataLen)
		{
			fprintf(stderr, "Not enough pressure data\n");
			goto fail;
		}
		for (unsigned int x = 0; x < blockW; x++)
		{
			for (unsigned int y = 0; y < blockH; y++)
			{
				i = pressData[j++];
				i2 = pressData[j++];
				pressure[blockY+y][blockX+x] = ((i+(i2<<8))/128.0f)-256;
			}
		}
	}

	//Read vx data
	if (vxData)
	{
		unsigned int j = 0;
		unsigned char i, i2;
		if (blockW * blockH > vxDataLen)
		{
			fprintf(stderr, "Not enough vx data\n");
			goto fail;
		}
		for (unsigned int x = 0; x < blockW; x++)
		{
			for (unsigned int y = 0; y < blockH; y++)
			{
				i = vxData[j++];
				i2 = vxData[j++];
				velocityX[blockY+y][blockX+x] = ((i+(i2<<8))/128.0f)-256;
			}
		}
	}

	//Read vy data
	if (vyData)
	{
		unsigned int j = 0;
		unsigned char i, i2;
		if (blockW * blockH > vyDataLen)
		{
			fprintf(stderr, "Not enough vy data\n");
			goto fail;
		}
		for (unsigned int x = 0; x < blockW; x++)
		{
			for (unsigned int y = 0; y < blockH; y++)
			{
				i = vyData[j++];
				i2 = vyData[j++];
				velocityY[blockY+y][blockX+x] = ((i+(i2<<8))/128.0f)-256;
			}
		}
	}

	//Read ambient data
	if (ambientData)
	{
		unsigned int i = 0, tempTemp;
		if (blockW * blockH > ambientDataLen)
		{
			fprintf(stderr, "Not enough ambient data\n");
			goto fail;
		}
		for (unsigned int x = 0; x < blockW; x++)
		{
			for (unsigned int y = 0; y < blockH; y++)
			{
				tempTemp = ambientData[i++];
				tempTemp |= (((unsigned)ambientData[i++]) << 8);
				ambientHeat[blockY+y][blockX+x] = tempTemp;
			}
		}
		hasAmbientHeat = true;
	}

	//Read particle data
	if (partsData && partsPosData)
	{
		int newIndex = 0, fieldDescriptor, tempTemp;
		int posCount, posTotal, partsPosDataIndex = 0;
		if (fullW * fullH * 3 > partsPosDataLen)
		{
			fprintf(stderr, "Not enough particle position data\n");
			goto fail;
		}

		partsSimIndex = (unsigned int*)calloc(NPART, sizeof(unsigned));
		partsCount = 0;

		unsigned int i = 0;
		unsigned int saved_x, saved_y, x, y;
		newIndex = 0;
		for (saved_y = 0; saved_y < fullH; saved_y++)
		{
			for (saved_x = 0; saved_x < fullW; saved_x++)
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
					if (newIndex >= NPART)
					{
						goto fail;
					}

					//i+3 because we have 4 bytes of required fields (type (1), descriptor (2), temp (1))
					if (i+3 >= partsDataLen)
						goto fail;
					x = saved_x + fullX;
					y = saved_y + fullY;
					fieldDescriptor = partsData[i+1];
					fieldDescriptor |= partsData[i+2] << 8;
					if (x >= fullW || y >= fullH)
					{
						fprintf(stderr, "Out of range [%d]: %d %d, [%d, %d], [%d, %d]\n", i, x, y, (unsigned)partsData[i+1], (unsigned)partsData[i+2], (unsigned)partsData[i+3], (unsigned)partsData[i+4]);
						goto fail;
					}

					if (newIndex < 0 || newIndex >= NPART)
						goto fail;

					//Store partsptr index+1 for this saved particle index (0 means not loaded)
					partsSimIndex[partsCount++] = newIndex+1;

					//Clear the particle, ready for our new properties
					memset(&(particles[newIndex]), 0, sizeof(Particle));

					//Required fields
					particles[newIndex].type = partsData[i];
					particles[newIndex].x = x;
					particles[newIndex].y = y;
					i+=3;

					//Read temp
					if(fieldDescriptor & 0x01)
					{
						//Full 16bit int
						tempTemp = partsData[i++];
						tempTemp |= (((unsigned)partsData[i++]) << 8);
						particles[newIndex].temp = tempTemp;
					}
					else
					{
						//1 Byte room temp offset
						tempTemp = (char)partsData[i++];
						particles[newIndex].temp = tempTemp+294.15f;
					}

					//Read life
					if(fieldDescriptor & 0x02)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].life = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x04)
						{
							if(i >= partsDataLen) goto fail;
							particles[newIndex].life |= (((unsigned)partsData[i++]) << 8);
						}
					}

					//Read tmp
					if(fieldDescriptor & 0x08)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].tmp = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x10)
						{
							if(i >= partsDataLen) goto fail;
							particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 8);
							//Read 3rd and 4th bytes
							if(fieldDescriptor & 0x1000)
							{
								if(i+1 >= partsDataLen) goto fail;
								particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 24);
								particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 16);
							}
						}
					}

					//Read ctype
					if(fieldDescriptor & 0x20)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].ctype = partsData[i++];
						//Read additional bytes
						if(fieldDescriptor & 0x200)
						{
							if(i+2 >= partsDataLen) goto fail;
							particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 24);
							particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 16);
							particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 8);
						}
					}

					//Read dcolour
					if(fieldDescriptor & 0x40)
					{
						if(i+3 >= partsDataLen) goto fail;
						particles[newIndex].dcolour = (((unsigned)partsData[i++]) << 24);
						particles[newIndex].dcolour |= (((unsigned)partsData[i++]) << 16);
						particles[newIndex].dcolour |= (((unsigned)partsData[i++]) << 8);
						particles[newIndex].dcolour |= ((unsigned)partsData[i++]);
					}

					//Read vx
					if(fieldDescriptor & 0x80)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].vx = (partsData[i++]-127.0f)/16.0f;
					}

					//Read vy
					if(fieldDescriptor & 0x100)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].vy = (partsData[i++]-127.0f)/16.0f;
					}

					//Read tmp2
					if(fieldDescriptor & 0x400)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].tmp2 = partsData[i++];
						if(fieldDescriptor & 0x800)
						{
							if(i >= partsDataLen) goto fail;
							particles[newIndex].tmp2 |= (((unsigned)partsData[i++]) << 8);
						}
					}

					//Read pavg
					if(fieldDescriptor & 0x2000)
					{
						if(i+3 >= partsDataLen) goto fail;
						int pavg;
						pavg = partsData[i++];
						pavg |= (((unsigned)partsData[i++]) << 8);
						particles[newIndex].pavg[0] = (float)pavg;
						pavg = partsData[i++];
						pavg |= (((unsigned)partsData[i++]) << 8);
						particles[newIndex].pavg[1] = (float)pavg;
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
							int caddress = restrict_flt(restrict_flt((float)(particles[newIndex].tmp-4), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
							particles[newIndex].type = PT_EMBR;
							particles[newIndex].tmp = 1;
							particles[newIndex].ctype = (((firw_data[caddress]))<<16) | (((firw_data[caddress+1]))<<8) | ((firw_data[caddress+2]));
						}
						break;
					case PT_PSTN:
						if (savedVersion < 87 && particles[newIndex].ctype)
							particles[newIndex].life = 1;
						if (savedVersion < 91)
							particles[newIndex].temp = 283.15;
						break;
					case PT_STKM:
					case PT_STKM2:
					case PT_FIGH:
						if (savedVersion < 88 && particles[newIndex].ctype == OLD_SPC_AIR)
							particles[newIndex].ctype = SPC_AIR;
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
					}
					//note: PSv was used in version 77.0 and every version before, add something in PSv too if the element is that old
					newIndex++;
				}
			}
		}
		if (soapLinkData)
		{
			unsigned int soapLinkDataPos = 0;
			for (unsigned int i = 0; i < partsCount; i++)
			{
				if (partsSimIndex[i] && particles[partsSimIndex[i]-1].type == PT_SOAP)
				{
					// Get the index of the particle forward linked from this one, if present in the save data
					unsigned int linkedIndex = 0;
					if (soapLinkDataPos+3 > soapLinkDataLen) break;
					linkedIndex |= soapLinkData[soapLinkDataPos++]<<16;
					linkedIndex |= soapLinkData[soapLinkDataPos++]<<8;
					linkedIndex |= soapLinkData[soapLinkDataPos++];
					// All indexes in soapLinkData and partsSimIndex have 1 added to them (0 means not saved/loaded)
					if (!linkedIndex || linkedIndex-1>=partsCount || !partsSimIndex[linkedIndex-1])
						continue;
					linkedIndex = partsSimIndex[linkedIndex-1]-1;
					newIndex = partsSimIndex[i]-1;

					//Attach the two particles
					particles[newIndex].ctype |= 2;
					particles[newIndex].tmp = linkedIndex;
					particles[linkedIndex].ctype |= 4;
					particles[linkedIndex].tmp2 = newIndex;
				}
			}
		}
	}

	if(tempSigns.size())
	{
		for (size_t i = 0; i < tempSigns.size(); i++)
		{
			if(signs.size() == MAXSIGNS)
				break;
			signs.push_back(tempSigns[i]);
		}
	}
	goto fin;
fail:
	//Clean up everything
	bson_destroy(&b);
	free(freeIndices);
	free(partsSimIndex);
	throw ParseException(ParseException::Corrupt, "Save data corrupt");
fin:
	bson_destroy(&b);
	free(freeIndices);
	free(partsSimIndex);
}

void GameSave::readPSv(char * data, int dataLength)
{
	unsigned char * d = NULL, * c = (unsigned char *)data;
	int q,i,j,k,x,y,p=0,*m=NULL, ver, pty, ty, legacy_beta=0;
	int bx0=0, by0=0, bw, bh, w, h, y0 = 0, x0 = 0;
	int new_format = 0, ttv = 0;
	int *fp = (int *)malloc(NPART*sizeof(int));

	std::vector<sign> tempSigns;
	char tempSignText[255];
	sign tempSign("", 0, 0, sign::Left);

	//Gol data used to read older saves
	int goltype[NGOL];
	int grule[NGOL+1][10];

	int golRulesCount;
	int * golRulesT = LoadGOLRules(golRulesCount);
	memcpy(grule, golRulesT, sizeof(int) * (golRulesCount*10));
	free(golRulesT);

	int golTypesCount;
	int * golTypesT = LoadGOLTypes(golTypesCount);
	memcpy(goltype, golTypesT, sizeof(int) * (golTypesCount));
	free(golTypesT);

	std::vector<Element> elements = GetElements();

	try
	{

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	if (dataLength<16)
		throw ParseException(ParseException::Corrupt, "No save data");
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		throw ParseException(ParseException::Corrupt, "Unknown format");
	if (c[2]==0x76 && c[1]==0x53 && c[0]==0x50) {
		new_format = 1;
	}
	if (c[4]>SAVE_VERSION)
		throw ParseException(ParseException::WrongVersion, "Save from newer version");
	ver = c[4];

	if (ver<34)
	{
		legacyEnable = 1;
	}
	else
	{
		if (ver>=44) {
			legacyEnable = c[3]&0x01;
			paused = (c[3]>>1)&0x01;
			if (ver>=46) {
				gravityMode = ((c[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				airMode = ((c[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49) {
				gravityEnable = ((c[3]>>7)&0x01);
			}
		} else {
			if (c[3]==1||c[3]==0) {
				legacyEnable = c[3];
			} else {
				legacy_beta = 1;
			}
		}
	}

	bw = c[6];
	bh = c[7];
	if (bx0+bw > XRES/CELL)
		bx0 = XRES/CELL - bw;
	if (by0+bh > YRES/CELL)
		by0 = YRES/CELL - bh;
	if (bx0 < 0)
		bx0 = 0;
	if (by0 < 0)
		by0 = 0;

	if (c[5]!=CELL || bx0+bw>XRES/CELL || by0+bh>YRES/CELL)
		throw ParseException(ParseException::InvalidDimensions, "Save too large");
	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	
	if(i > 209715200 || !i)
		throw ParseException(ParseException::InvalidDimensions, "Save data too large");
	
	d = (unsigned char *)malloc(i);
	if (!d)
		throw ParseException(ParseException::Corrupt, "Cannot allocate memory");

	setSize(bw, bh);

	int bzStatus = 0;
	if ((bzStatus = BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), dataLength-12, 0, 0)))
	{
		std::stringstream bzStatusStr;
		bzStatusStr << bzStatus;
		throw ParseException(ParseException::Corrupt, "Cannot decompress: " + bzStatusStr.str());
	}
	dataLength = i;

#ifdef DEBUG
	std::cout << "Parsing " << dataLength << " bytes of data, version " << ver << std::endl;
#endif

	if (dataLength < bw*bh)
		throw ParseException(ParseException::Corrupt, "Save data corrupt (missing data)");

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	if (ver<46) {
		gravityMode = 0;
		airMode = 0;
	}
	m = (int *)calloc(XRES*YRES, sizeof(int));

	// load the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
		{
			if (d[p])
			{
				//In old saves, ignore walls created by sign tool bug
				//Not ignoring other invalid walls or invalid walls in new saves, so that any other bugs causing them are easier to notice, find and fix
				if (ver>=44 && ver<71 && d[p]==O_WL_SIGN)
				{
					p++;
					continue;
				}
				blockMap[y][x] = d[p];
				if (blockMap[y][x]==1)
					blockMap[y][x]=WL_WALL;
				else if (blockMap[y][x]==2)
					blockMap[y][x]=WL_DESTROYALL;
				else if (blockMap[y][x]==3)
					blockMap[y][x]=WL_ALLOWLIQUID;
				else if (blockMap[y][x]==4)
					blockMap[y][x]=WL_FAN;
				else if (blockMap[y][x]==5)
					blockMap[y][x]=WL_STREAM;
				else if (blockMap[y][x]==6)
					blockMap[y][x]=WL_DETECT;
				else if (blockMap[y][x]==7)
					blockMap[y][x]=WL_EWALL;
				else if (blockMap[y][x]==8)
					blockMap[y][x]=WL_WALLELEC;
				else if (blockMap[y][x]==9)
					blockMap[y][x]=WL_ALLOWAIR;
				else if (blockMap[y][x]==10)
					blockMap[y][x]=WL_ALLOWPOWDER;
				else if (blockMap[y][x]==11)
					blockMap[y][x]=WL_ALLOWALLELEC;
				else if (blockMap[y][x]==12)
					blockMap[y][x]=WL_EHOLE;
				else if (blockMap[y][x]==13)
					blockMap[y][x]=WL_ALLOWGAS;

				if (ver>=44)
				{
					/* The numbers used to save walls were changed, starting in v44.
					 * The new numbers are ignored for older versions due to some corruption of bmap in saves from older versions. 
					 */
					if (blockMap[y][x]==O_WL_WALLELEC)
						blockMap[y][x]=WL_WALLELEC;
					else if (blockMap[y][x]==O_WL_EWALL)
						blockMap[y][x]=WL_EWALL;
					else if (blockMap[y][x]==O_WL_DETECT)
						blockMap[y][x]=WL_DETECT;
					else if (blockMap[y][x]==O_WL_STREAM)
						blockMap[y][x]=WL_STREAM;
					else if (blockMap[y][x]==O_WL_FAN||blockMap[y][x]==O_WL_FANHELPER)
						blockMap[y][x]=WL_FAN;
					else if (blockMap[y][x]==O_WL_ALLOWLIQUID)
						blockMap[y][x]=WL_ALLOWLIQUID;
					else if (blockMap[y][x]==O_WL_DESTROYALL)
						blockMap[y][x]=WL_DESTROYALL;
					else if (blockMap[y][x]==O_WL_ERASE)
						blockMap[y][x]=WL_ERASE;
					else if (blockMap[y][x]==O_WL_WALL)
						blockMap[y][x]=WL_WALL;
					else if (blockMap[y][x]==O_WL_ALLOWAIR)
						blockMap[y][x]=WL_ALLOWAIR;
					else if (blockMap[y][x]==O_WL_ALLOWSOLID)
						blockMap[y][x]=WL_ALLOWPOWDER;
					else if (blockMap[y][x]==O_WL_ALLOWALLELEC)
						blockMap[y][x]=WL_ALLOWALLELEC;
					else if (blockMap[y][x]==O_WL_EHOLE)
						blockMap[y][x]=WL_EHOLE;
					else if (blockMap[y][x]==O_WL_ALLOWGAS)
						blockMap[y][x]=WL_ALLOWGAS;
					else if (blockMap[y][x]==O_WL_GRAV)
						blockMap[y][x]=WL_GRAV;
					else if (blockMap[y][x]==O_WL_ALLOWENERGY)
						blockMap[y][x]=WL_ALLOWENERGY;
				}

				if (blockMap[y][x] >= UI_WALLCOUNT)
					blockMap[y][x] = 0;
			}

			p++;
		}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||(ver>=44 && d[(y-by0)*bw+(x-bx0)]==O_WL_FAN))
			{
				if (p >= dataLength)
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				fanVelX[y][x] = (d[p++]-127.0f)/64.0f;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||(ver>=44 && d[(y-by0)*bw+(x-bx0)]==O_WL_FAN))
			{
				if (p >= dataLength)
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				fanVelY[y][x] = (d[p++]-127.0f)/64.0f;
			}

	// load the particle map
	i = 0;
	k = 0;
	pty = p;
	for (y=y0; y<y0+h; y++)
		for (x=x0; x<x0+w; x++)
		{
			if (p >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			j=d[p++];
			if (j >= PT_NUM) {
				//TODO: Possibly some server side translation
				j = PT_DUST;//throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			}
			if (j)
			{
				memset(particles+k, 0, sizeof(Particle));
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
				particles[k].x = (float)x;
				particles[k].y = (float)y;
				m[(x-x0)+(y-y0)*w] = k+1;
				particlesCount = ++k;
			}
		}

	// load particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			if (p+1 >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			if (i < NPART)
			{
				particles[i].vx = (d[p++]-127.0f)/16.0f;
				particles[i].vy = (d[p++]-127.0f)/16.0f;
			}
			else
				p += 2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=44) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					particles[i-1].life = ttv;
				} else {
					p+=2;
				}
			} else {
				if (p >= dataLength)
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				if (i <= NPART)
					particles[i-1].life = d[p++]*4;
				else
					p++;
			}
		}
	}
	if (ver>=44) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i)
			{
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					particles[i-1].tmp = ttv;
					if (ver<53 && !particles[i-1].tmp)
						for (q = 1; q<=NGOL; q++) {
							if (particles[i-1].type==goltype[q-1] && grule[q][9]==2)
								particles[i-1].tmp = grule[q][9]-1;
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
	if (ver>=53) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			ty = d[pty+j];
			if (i && (ty==PT_PBCN || (ty==PT_TRON && ver>=77)))
			{
				if (p >= dataLength)
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				if (i <= NPART)
					particles[i-1].tmp2 = d[p++];
				else
					p++;
			}
		}
	}
	//Read ALPHA component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour = d[p++]<<24;
				} else {
					p++;
				}
			}
		}
	}
	//Read RED component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= d[p++]<<16;
				} else {
					p++;
				}
			}
		}
	}
	//Read GREEN component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= d[p++]<<8;
				} else {
					p++;
				}
			}
		}
	}
	//Read BLUE component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= d[p++];
				} else {
					p++;
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		ty = d[pty+j];
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
							ttv = (d[p++])<<8;
							ttv |= (d[p++]);
							if (particles[i-1].type==PT_PUMP) {
								particles[i-1].temp = ttv + 0.15;//fix PUMP saved at 0, so that it loads at 0.
							} else {
								particles[i-1].temp = ttv;
							}
						} else {
							particles[i-1].temp = (d[p++]*((MAX_TEMP+(-MIN_TEMP))/255))+MIN_TEMP;
						}
					} else {
						particles[i-1].temp = ((d[p++]*((O_MAX_TEMP+(-O_MIN_TEMP))/255))+O_MIN_TEMP)+273;
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
				particles[i-1].temp = elements[particles[i-1].type].Temperature;
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		int gnum = 0;
		i = m[j];
		ty = d[pty+j];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43) || (ty==PT_LIFE && ver>=51) || (ty==PT_PBCN && ver>=52) || (ty==PT_WIRE && ver>=55) || (ty==PT_STOR && ver>=59) || (ty==PT_CONV && ver>=60)))
		{
			if (p >= dataLength)
				throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
			if (i <= NPART)
				particles[i-1].ctype = d[p++];
			else
				p++;
		}
		//TODO: STKM_init_legs
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
				x = (int)(particles[i-1].x+0.5f);
				y = (int)(particles[i-1].y+0.5f);
				particles[i-1].dcolour = 0xFF000000;
				particles[i-1].type = PT_DMND;
			}
			if(ver<51 && ((ty>=78 && ty<=89) || (ty>=134 && ty<=146 && ty!=141))){
				//Replace old GOL
				particles[i-1].type = PT_LIFE;
				for (gnum = 0; gnum<NGOL; gnum++){
					if (ty==goltype[gnum])
						particles[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOL; gnum++){
					if (particles[i-1].ctype==goltype[gnum])
					{
						particles[i-1].ctype = PT_LIFE;
						particles[i-1].tmp = gnum;
					}
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
					int caddress = restrict_flt(restrict_flt((float)(particles[i-1].tmp-4), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
					particles[i-1].type = PT_EMBR;
					particles[i-1].tmp = 1;
					particles[i-1].ctype = (((firw_data[caddress]))<<16) | (((firw_data[caddress+1]))<<8) | ((firw_data[caddress+2]));
				}
			}
			if (ver < 88) //fix air blowing stickmen
				if ((particles[i-1].type == PT_STKM || particles[i-1].type == PT_STKM2 || particles[i-1].type == PT_FIGH) && particles[i-1].ctype == OLD_SPC_AIR)
					particles[i-1].ctype = SPC_AIR;
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
		}
	}

	if (p >= dataLength)
		goto version1;
	j = d[p++];
	for (i=0; i<j; i++)
	{
		if (p+6 > dataLength)
			throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		tempSign.x = x+x0;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		tempSign.y = x+y0;
		x = d[p++];
		tempSign.ju = (sign::Justification)x;
		x = d[p++];
		if (p+x > dataLength)
			throw ParseException(ParseException::Corrupt, "Not enough data at line " MTOS(__LINE__) " in " MTOS(__FILE__));
		if(x>254)
			x = 254;
		memcpy(tempSignText, d+p, x);
		tempSignText[x] = 0;
		tempSign.text = format::CleanString(tempSignText, true, true, true).substr(0, 45);
		tempSigns.push_back(tempSign);
		p += x;
	}

	for (size_t i = 0; i < tempSigns.size(); i++)
	{
		if(signs.size() == MAXSIGNS)
			break;
		signs.push_back(tempSigns[i]);
	}

	}
	catch(ParseException & e)
	{
		if (m)
		{
			free(m);
			m = 0;
		}
		if (d)
		{
			free(d);
			d = 0;
		}
		if (fp)
		{
			free(fp);
			fp = 0;
		}
		throw;
	}

	version1:
	if (m)
	{
		free(m);
		m = 0;
	}
	if (d)
	{
		free(d);
		d = 0;
	}
	if (fp)
	{
		free(fp);
		fp = 0;
	}
}

// restrict the minimum version this save can be opened with
#define RESTRICTVERSION(major, minor) if ((major) > minimumMajorVersion || (((major) == minimumMajorVersion && (minor) > minimumMinorVersion))) {\
	minimumMajorVersion = major;\
	minimumMinorVersion = minor;\
}

char * GameSave::serialiseOPS(unsigned int & dataLength)
{
	//Particle *particles = sim->parts;
	unsigned char *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL, *finalData = NULL, *outputData = NULL, *soapLinkData = NULL;
	unsigned char *pressData = NULL, *vxData = NULL, *vyData = NULL, *ambientData = NULL;
	unsigned *partsPosLink = NULL, *partsPosFirstMap = NULL, *partsPosCount = NULL, *partsPosLastMap = NULL;
	unsigned partsCount = 0, *partsSaveIndex = NULL;
	unsigned *elementCount = new unsigned[PT_NUM];
	unsigned int partsDataLen, partsPosDataLen, fanDataLen, wallDataLen,finalDataLen, outputDataLen, soapLinkDataLen;
	unsigned int pressDataLen = 0, vxDataLen = 0, vyDataLen = 0, ambientDataLen = 0;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	int x, y, i, wallDataFound = 0;
	int posCount, signsCount;
	// minimum version this save is compatible with
	// when building, this number may be increased depending on what elements are used
	// or what properties are detected
	int minimumMajorVersion = 90, minimumMinorVersion = 2;
	bson b;

	std::fill(elementCount, elementCount+PT_NUM, 0);

	//Get coords in blocks
	blockX = 0;//orig_x0/CELL;
	blockY = 0;//orig_y0/CELL;

	//Snap full coords to block size
	fullX = blockX*CELL;
	fullY = blockY*CELL;

	//Original size + offset of original corner from snapped corner, rounded up by adding CELL-1
	blockW = blockWidth;//(blockWidth-fullX+CELL-1)/CELL;
	blockH = blockHeight;//(blockHeight-fullY+CELL-1)/CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;

	//Copy fan and wall data
	wallData = (unsigned char*)malloc(blockW*blockH);
	wallDataLen = blockW*blockH;
	fanData = (unsigned char*)malloc((blockW*blockH)*2);
	fanDataLen = 0;
	pressData = (unsigned char*)malloc((blockW*blockH)*2);
	vxData = (unsigned char*)malloc((blockW*blockH)*2);
	vyData = (unsigned char*)malloc((blockW*blockH)*2);
	ambientData = (unsigned char*)malloc((blockW*blockH)*2);
	if (!wallData || !fanData || !pressData || !vxData || !vyData || !ambientData)
	{
		puts("Save Error, out of memory\n");
		outputData = NULL;
		goto fin;
	}
	for(x = blockX; x < blockX+blockW; x++)
	{
		for(y = blockY; y < blockY+blockH; y++)
		{
			wallData[(y-blockY)*blockW+(x-blockX)] = blockMap[y][x];

			//save pressure and x/y velocity grids
			float pres = std::max(-255.0f,std::min(255.0f,pressure[y][x]))+256.0f;
			float velX = std::max(-255.0f,std::min(255.0f,velocityX[y][x]))+256.0f;
			float velY = std::max(-255.0f,std::min(255.0f,velocityY[y][x]))+256.0f;
			pressData[pressDataLen++] = (unsigned char)((int)(pres*128)&0xFF);
			pressData[pressDataLen++] = (unsigned char)((int)(pres*128)>>8);

			vxData[vxDataLen++] = (unsigned char)((int)(velX*128)&0xFF);
			vxData[vxDataLen++] = (unsigned char)((int)(velX*128)>>8);

			vyData[vyDataLen++] = (unsigned char)((int)(velY*128)&0xFF);
			vyData[vyDataLen++] = (unsigned char)((int)(velY*128)>>8);

			int tempTemp = (int)(ambientHeat[y][x]+0.5f);
			ambientData[ambientDataLen++] = tempTemp;
			ambientData[ambientDataLen++] = tempTemp >> 8;

			if(blockMap[y][x] && !wallDataFound)
				wallDataFound = 1;
			if(blockMap[y][x]==WL_FAN)
			{
				i = (int)(fanVelX[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
				i = (int)(fanVelY[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
			}
		}
	}
	if(!fanDataLen)
	{
		free(fanData);
		fanData = NULL;
	}
	if(!wallDataFound)
	{
		free(wallData);
		wallData = NULL;
	}

	//Index positions of all particles, using linked lists
	//partsPosFirstMap is pmap for the first particle in each position
	//partsPosLastMap is pmap for the last particle in each position
	//partsPosCount is the number of particles in each position
	//partsPosLink contains, for each particle, (i<<8)|1 of the next particle in the same position
	partsPosFirstMap = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosLastMap = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosCount = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosLink = (unsigned int *)calloc(NPART, sizeof(unsigned));
	for(i = 0; i < particlesCount; i++)
	{
		if(particles[i].type)
		{
			x = (int)(particles[i].x+0.5f);
			y = (int)(particles[i].y+0.5f);
			//Coordinates relative to top left corner of saved area
			x -= fullX;
			y -= fullY;
			if (!partsPosFirstMap[y*fullW + x])
			{
				//First entry in list
				partsPosFirstMap[y*fullW + x] = (i<<8)|1;
				partsPosLastMap[y*fullW + x] = (i<<8)|1;
			}
			else
			{
				//Add to end of list
				partsPosLink[partsPosLastMap[y*fullW + x]>>8] = (i<<8)|1;//link to current end of list
				partsPosLastMap[y*fullW + x] = (i<<8)|1;//set as new end of list
			}
			partsPosCount[y*fullW + x]++;
		}
	}

	//Store number of particles in each position
	partsPosData = (unsigned char*)malloc(fullW*fullH*3);
	partsPosDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			posCount = partsPosCount[y*fullW + x];
			partsPosData[partsPosDataLen++] = (posCount&0x00FF0000)>>16;
			partsPosData[partsPosDataLen++] = (posCount&0x0000FF00)>>8;
			partsPosData[partsPosDataLen++] = (posCount&0x000000FF);
		}
	}

	//Copy parts data
	/* Field descriptor format:
	 |		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|
									|	   pavg		|	 tmp[3+4]	|	tmp2[2]		|		tmp2	|	ctype[2]	|		vy		|		vx		|	dcololour	|	ctype[1]	|		tmp[2]	|		tmp[1]	|		life[2]	|		life[1]	|	temp dbl len|
	 life[2] means a second byte (for a 16 bit field) if life[1] is present
	 */
	partsData = (unsigned char *)malloc(NPART * (sizeof(Particle)+1));
	partsDataLen = 0;
	partsSaveIndex = (unsigned int *)calloc(NPART, sizeof(unsigned));
	partsCount = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			//Find the first particle in this position
			i = partsPosFirstMap[y*fullW + x];

			//Loop while there is a pmap entry
			while (i)
			{
				unsigned short fieldDesc = 0;
				int fieldDescLoc = 0, tempTemp, vTemp;

				//Turn pmap entry into a particles index
				i = i>>8;

				//Store saved particle index+1 for this partsptr index (0 means not saved)
				partsSaveIndex[i] = (partsCount++) + 1;

				//Type (required)
				partsData[partsDataLen++] = particles[i].type;
				elementCount[particles[i].type]++;

				//Location of the field descriptor
				fieldDescLoc = partsDataLen++;
				partsDataLen++;

				//Extra Temperature (2nd byte optional, 1st required), 1 to 2 bytes
				//Store temperature as an offset of 21C(294.15K) or go into a 16byte int and store the whole thing
				if(fabs(particles[i].temp-294.15f)<127)
				{
					tempTemp = floor(particles[i].temp-294.15f+0.5f);
					partsData[partsDataLen++] = tempTemp;
				}
				else
				{
					fieldDesc |= 1;
					tempTemp = (int)(particles[i].temp+0.5f);
					partsData[partsDataLen++] = tempTemp;
					partsData[partsDataLen++] = tempTemp >> 8;
				}

				//Life (optional), 1 to 2 bytes
				if(particles[i].life)
				{
					int life = particles[i].life;
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
				if(particles[i].tmp)
				{
					fieldDesc |= 1 << 3;
					partsData[partsDataLen++] = particles[i].tmp;
					if(particles[i].tmp & 0xFFFFFF00)
					{
						fieldDesc |= 1 << 4;
						partsData[partsDataLen++] = particles[i].tmp >> 8;
						if(particles[i].tmp & 0xFFFF0000)
						{
							fieldDesc |= 1 << 12;
							partsData[partsDataLen++] = (particles[i].tmp&0xFF000000)>>24;
							partsData[partsDataLen++] = (particles[i].tmp&0x00FF0000)>>16;
						}
					}
				}

				//Ctype (optional), 1 or 4 bytes
				if(particles[i].ctype)
				{
					fieldDesc |= 1 << 5;
					partsData[partsDataLen++] = particles[i].ctype;
					if(particles[i].ctype & 0xFFFFFF00)
					{
						fieldDesc |= 1 << 9;
						partsData[partsDataLen++] = (particles[i].ctype&0xFF000000)>>24;
						partsData[partsDataLen++] = (particles[i].ctype&0x00FF0000)>>16;
						partsData[partsDataLen++] = (particles[i].ctype&0x0000FF00)>>8;
					}
				}

				//Dcolour (optional), 4 bytes
				if(particles[i].dcolour && (particles[i].dcolour & 0xFF000000))
				{
					fieldDesc |= 1 << 6;
					partsData[partsDataLen++] = (particles[i].dcolour&0xFF000000)>>24;
					partsData[partsDataLen++] = (particles[i].dcolour&0x00FF0000)>>16;
					partsData[partsDataLen++] = (particles[i].dcolour&0x0000FF00)>>8;
					partsData[partsDataLen++] = (particles[i].dcolour&0x000000FF);
				}

				//VX (optional), 1 byte
				if(fabs(particles[i].vx) > 0.001f)
				{
					fieldDesc |= 1 << 7;
					vTemp = (int)(particles[i].vx*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}

				//VY (optional), 1 byte
				if(fabs(particles[i].vy) > 0.001f)
				{
					fieldDesc |= 1 << 8;
					vTemp = (int)(particles[i].vy*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}

				//Tmp2 (optional), 1 or 2 bytes
				if(particles[i].tmp2)
				{
					fieldDesc |= 1 << 10;
					partsData[partsDataLen++] = particles[i].tmp2;
					if(particles[i].tmp2 & 0xFF00)
					{
						fieldDesc |= 1 << 11;
						partsData[partsDataLen++] = particles[i].tmp2 >> 8;
					}
				}

				//Pavg, 4 bytes
				//Don't save pavg for things that break under pressure, because then they will break when the save is loaded, since pressure isn't also loaded
				if ((particles[i].pavg[0] || particles[i].pavg[1]) && !(particles[i].type == PT_QRTZ || particles[i].type == PT_GLAS || particles[i].type == PT_TUNG))
				{
					fieldDesc |= 1 << 13;
					partsData[partsDataLen++] = (int)particles[i].pavg[0];
					partsData[partsDataLen++] = ((int)particles[i].pavg[0])>>8;
					partsData[partsDataLen++] = (int)particles[i].pavg[1];
					partsData[partsDataLen++] = ((int)particles[i].pavg[1])>>8;
				}

				//Write the field descriptor
				partsData[fieldDescLoc] = fieldDesc;
				partsData[fieldDescLoc+1] = fieldDesc>>8;

				if (particles[i].type == PT_RPEL && particles[i].ctype)
				{
					RESTRICTVERSION(91, 4);
				}
				else if (particles[i].type == PT_NWHL && particles[i].tmp)
				{
					RESTRICTVERSION(91, 5);
				}
				if (particles[i].type == PT_HEAC || particles[i].type == PT_SAWD || particles[i].type == PT_POLO
						|| particles[i].type == PT_RFRG || particles[i].type == PT_RFGL || particles[i].type == PT_LSNS)
				{
					RESTRICTVERSION(92, 0);
					fromNewerVersion = true;
				}
				else if ((particles[i].type == PT_FRAY || particles[i].type == PT_INVIS) && particles[i].tmp)
				{
					RESTRICTVERSION(92, 0);
					fromNewerVersion = true;
				}

				//Get the pmap entry for the next particle in the same position
				i = partsPosLink[i];
			}
		}
	}

	soapLinkData = (unsigned char*)malloc(3*elementCount[PT_SOAP]);
	soapLinkDataLen = 0;
	//Iterate through particles in the same order that they were saved
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			//Find the first particle in this position
			i = partsPosFirstMap[y*fullW + x];

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
	if(!soapLinkDataLen)
	{
		free(soapLinkData);
		soapLinkData = NULL;
	}
	if(!partsDataLen)
	{
		free(partsData);
		partsData = NULL;
	}

	bson_init(&b);
	bson_append_start_object(&b, "origin");
	bson_append_int(&b, "majorVersion", SAVE_VERSION);
	bson_append_int(&b, "minorVersion", MINOR_VERSION);
	bson_append_int(&b, "buildNum", BUILD_NUM);
	bson_append_int(&b, "snapshotId", SNAPSHOT_ID);
	bson_append_int(&b, "modId", MOD_ID);
	bson_append_string(&b, "releaseType", IDENT_RELTYPE);
	bson_append_string(&b, "platform", IDENT_PLATFORM);
	bson_append_string(&b, "builtType", IDENT_BUILD);
	bson_append_finish_object(&b);
	bson_append_start_object(&b, "minimumVersion");
	bson_append_int(&b, "major", minimumMajorVersion);
	bson_append_int(&b, "minor", minimumMinorVersion);
	bson_append_finish_object(&b);
	

	bson_append_bool(&b, "waterEEnabled", waterEEnabled);
	bson_append_bool(&b, "legacyEnable", legacyEnable);
	bson_append_bool(&b, "gravityEnable", gravityEnable);
	bson_append_bool(&b, "aheat_enable", aheatEnable);
	bson_append_bool(&b, "paused", paused);
	bson_append_int(&b, "gravityMode", gravityMode);
	bson_append_int(&b, "airMode", airMode);
	bson_append_int(&b, "edgeMode", edgeMode);

	//bson_append_int(&b, "leftSelectedElement", sl);
	//bson_append_int(&b, "rightSelectedElement", sr);
	//bson_append_int(&b, "activeMenu", active_menu);
	if (partsData)
		bson_append_binary(&b, "parts", BSON_BIN_USER, (const char *)partsData, partsDataLen);
	if (partsPosData)
		bson_append_binary(&b, "partsPos", BSON_BIN_USER, (const char *)partsPosData, partsPosDataLen);
	if (wallData)
		bson_append_binary(&b, "wallMap", BSON_BIN_USER, (const char *)wallData, wallDataLen);
	if (fanData)
		bson_append_binary(&b, "fanMap", BSON_BIN_USER, (const char *)fanData, fanDataLen);
	if (pressData)
		bson_append_binary(&b, "pressMap", (char)BSON_BIN_USER, (const char*)pressData, pressDataLen);
	if (vxData)
		bson_append_binary(&b, "vxMap", (char)BSON_BIN_USER, (const char*)vxData, vxDataLen);
	if (vyData)
		bson_append_binary(&b, "vyMap", (char)BSON_BIN_USER, (const char*)vyData, vyDataLen);
	if (ambientData && this->aheatEnable)
		bson_append_binary(&b, "ambientMap", (char)BSON_BIN_USER, (const char*)ambientData, ambientDataLen);
	if (soapLinkData)
		bson_append_binary(&b, "soapLinks", BSON_BIN_USER, (const char *)soapLinkData, soapLinkDataLen);
	if (partsData && palette.size())
	{
		bson_append_start_array(&b, "palette");
		for(std::vector<PaletteItem>::iterator iter = palette.begin(), end = palette.end(); iter != end; ++iter)
		{
			bson_append_int(&b, (*iter).first.c_str(), (*iter).second);
		}
		bson_append_finish_array(&b);
	}
	signsCount = 0;
	for (size_t i = 0; i < signs.size(); i++)
	{
		if(signs[i].text.length() && signs[i].x>=0 && signs[i].x<=fullW && signs[i].y>=0 && signs[i].y<=fullH)
		{
			signsCount++;
		}
	}
	if (signsCount)
	{
		bson_append_start_array(&b, "signs");
		for (size_t i = 0; i < signs.size(); i++)
		{
			if(signs[i].text.length() && signs[i].x>=0 && signs[i].x<=fullW && signs[i].y>=0 && signs[i].y<=fullH)
			{
				bson_append_start_object(&b, "sign");
				bson_append_string(&b, "text", signs[i].text.c_str());
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
		goto fin;

	finalData = (unsigned char *)bson_data(&b);
	finalDataLen = bson_size(&b);
	outputDataLen = finalDataLen*2+12;
	outputData = new unsigned char[outputDataLen];

	outputData[0] = 'O';
	outputData[1] = 'P';
	outputData[2] = 'S';
	outputData[3] = '1';
	outputData[4] = SAVE_VERSION;
	outputData[5] = CELL;
	outputData[6] = blockW;
	outputData[7] = blockH;
	outputData[8] = finalDataLen;
	outputData[9] = finalDataLen >> 8;
	outputData[10] = finalDataLen >> 16;
	outputData[11] = finalDataLen >> 24;

	if (BZ2_bzBuffToBuffCompress((char*)(outputData+12), &outputDataLen, (char*)finalData, bson_size(&b), 9, 0, 0) != BZ_OK)
	{
		puts("Save Error\n");
		delete [] outputData;
		dataLength = 0;
		outputData = NULL;
		goto fin;
	}

#ifdef DEBUG
	printf("compressed data: %d\n", outputDataLen);
#endif
	dataLength = outputDataLen + 12;

fin:
	bson_destroy(&b);
	free(partsData);
	free(wallData);
	free(pressData);
	free(vxData);
	free(vyData);
	free(ambientData);
	free(fanData);
	delete[] elementCount;
	free(partsSaveIndex);
	free(soapLinkData);
	free(partsPosData);
	free(partsPosFirstMap);
	free(partsPosLastMap);
	free(partsPosCount);
	free(partsPosLink);

	return (char*)outputData;
}

void GameSave::ConvertBsonToJson(bson_iterator *iter, Json::Value *j, int depth)
{
	bson_iterator subiter;
	bson_iterator_subiterator(iter, &subiter);
	while (bson_iterator_next(&subiter))
	{
		std::string key = bson_iterator_key(&subiter);
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
		std::string member = *iter;
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
void GameSave::ConvertJsonToBson(bson *b, Json::Value j, int depth)
{
	Json::Value::Members members = j.getMemberNames();
	for (Json::Value::Members::iterator iter = members.begin(), end = members.end(); iter != end; ++iter)
	{
		std::string member = *iter;
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

// deallocates a pointer to a 2D array and sets it to NULL
template <typename T>
void GameSave::Deallocate2DArray(T ***array, int blockHeight)
{
	if (*array)
	{
		for (int y = 0; y < blockHeight; y++)
			delete[] (*array)[y];
		delete[] (*array);
		*array = NULL;
	}
}

void GameSave::dealloc()
{
	if (particles)
	{
		delete[] particles;
		particles = NULL;
	}
	Deallocate2DArray<unsigned char>(&blockMap, blockHeight);
	Deallocate2DArray<float>(&fanVelX, blockHeight);
	Deallocate2DArray<float>(&fanVelY, blockHeight);
	Deallocate2DArray<float>(&pressure, blockHeight);
	Deallocate2DArray<float>(&velocityX, blockHeight);
	Deallocate2DArray<float>(&velocityY, blockHeight);
	Deallocate2DArray<float>(&ambientHeat, blockHeight);
}

GameSave::~GameSave()
{
	dealloc();
}
