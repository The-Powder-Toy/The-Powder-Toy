#include "Simulation.h"
#include "Air.h"
#include "ElementClasses.h"
#include "gravity/Gravity.h"
#include "ToolClasses.h"
#include "SimulationData.h"
#include "client/GameSave.h"
#include "common/tpt-compat.h"
#include "common/tpt-rand.h"
#include "common/tpt-thread-local.h"
#include "gui/game/Brush.h"
#include "elements/EMP.h"
#include "elements/LOLZ.h"
#include "elements/STKM.h"
#include "elements/PIPE.h"
#include "elements/FILT.h"
#include <iostream>
#include <set>

static float remainder_p(float x, float y)
{
	return std::fmod(x, y) + (x>=0 ? 0 : y);
}

void Simulation::Load(const GameSave *save, bool includePressure, Vec2<int> blockP) // block coordinates
{
	auto partP = blockP * CELL;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;

	RecalcFreeParticles(false);

	struct ExistingParticle
	{
		int id;
		Vec2<int> pos;
	};
	std::vector<ExistingParticle> existingParticles;
	auto pasteArea = RES.OriginRect() & RectSized(partP, save->blockSize * CELL);
	for (int i = 0; i <= parts_lastActiveIndex; i++)
	{
		if (parts[i].type)
		{
			auto p = Vec2<int>{ int(parts[i].x + 0.5f), int(parts[i].y + 0.5f) };
			if (pasteArea.Contains(p))
			{
				existingParticles.push_back({ i, p });
			}
		}
	}
	std::sort(existingParticles.begin(), existingParticles.end(), [](const auto &lhs, const auto &rhs) {
		return std::tie(lhs.pos.Y, lhs.pos.X) < std::tie(rhs.pos.Y, rhs.pos.X);
	});
	PlaneAdapter<std::vector<size_t>> existingParticleIndices(pasteArea.Size(), existingParticles.size());
	{
		auto lastPos = Vec2<int>{ -1, -1 }; // not a valid pos in existingParticles
		for (auto it = existingParticles.begin(); it != existingParticles.end(); ++it)
		{
			if (lastPos != it->pos)
			{
				existingParticleIndices[it->pos - pasteArea.TopLeft] = it - existingParticles.begin();
				lastPos = it->pos;
			}
		}
	}
	auto removeExistingParticles = [this, pasteArea, &existingParticles, &existingParticleIndices](Vec2<int> p) {
		auto rp = p - pasteArea.TopLeft;
		if (existingParticleIndices.Size().OriginRect().Contains(rp))
		{
			auto index = existingParticleIndices[rp];
			for (auto it = existingParticles.begin() + index; it != existingParticles.end() && it->pos == p; ++it)
			{
				kill_part(it->id);
			}
			existingParticleIndices[rp] = existingParticles.size();
		}
	};

	std::map<unsigned int, unsigned int> soapList;
	for (int n = 0; n < NPART && n < save->particlesCount; n++)
	{
		Particle tempPart = save->particles[n];
		if (tempPart.type <= 0 || tempPart.type >= PT_NUM)
		{
			continue;
		}

		tempPart.x += (float)partP.X;
		tempPart.y += (float)partP.Y;
		int x = int(std::floor(tempPart.x + 0.5f));
		int y = int(std::floor(tempPart.y + 0.5f));

		// Check various scenarios where we are unable to spawn the element, and set type to 0 to block spawning later
		if (!InBounds(x, y))
		{
			continue;
		}

		// Ensure we can spawn this element
		if ((player.spwn == 1 && tempPart.type==PT_STKM) || (player2.spwn == 1 && tempPart.type==PT_STKM2))
		{
			continue;
		}
		if ((tempPart.type == PT_SPAWN || tempPart.type == PT_SPAWN2) && elementCount[tempPart.type])
		{
			continue;
		}
		if (tempPart.type == PT_FIGH && !Element_FIGH_CanAlloc(this))
		{
			continue;
		}
		if (!elements[tempPart.type].Enabled)
		{
			continue;
		}

		if (elements[tempPart.type].CreateAllowed)
		{
			if (!(*(elements[tempPart.type].CreateAllowed))(this, -3, int(tempPart.x + 0.5f), int(tempPart.y + 0.5f), tempPart.type))
			{
				continue;
			}
		}

		removeExistingParticles({ x, y });

		// Allocate particle (this location is guaranteed to be empty due to "full scan" logic above)
		if (pfree == -1)
			break;
		auto i = pfree;
		pfree = parts[i].life;
		if (i > parts_lastActiveIndex)
			parts_lastActiveIndex = i;
		parts[i] = tempPart;
		elementCount[tempPart.type]++;


		switch (parts[i].type)
		{
		case PT_STKM:
			Element_STKM_init_legs(this, &player, i);
			player.spwn = 1;
			player.elem = PT_DUST;

			if ((save->version < Version(93, 0) && parts[i].ctype == SPC_AIR) ||
			        (save->version < Version(88, 0) && parts[i].ctype == OLD_SPC_AIR))
			{
				player.fan = true;
			}
			if (save->stkm.rocketBoots1)
				player.rocketBoots = true;
			if (save->stkm.fan1)
				player.fan = true;
			break;
		case PT_STKM2:
			Element_STKM_init_legs(this, &player2, i);
			player2.spwn = 1;
			player2.elem = PT_DUST;
			if ((save->version < Version(93, 0) && parts[i].ctype == SPC_AIR) ||
			        (save->version < Version(88, 0) && parts[i].ctype == OLD_SPC_AIR))
			{
				player2.fan = true;
			}
			if (save->stkm.rocketBoots2)
				player2.rocketBoots = true;
			if (save->stkm.fan2)
				player2.fan = true;
			break;
		case PT_SPAWN:
			player.spawnID = i;
			break;
		case PT_SPAWN2:
			player2.spawnID = i;
			break;
		case PT_FIGH:
		{
			unsigned int oldTmp = parts[i].tmp;
			parts[i].tmp = Element_FIGH_Alloc(this);
			if (parts[i].tmp >= 0)
			{
				bool fan = false;
				if ((save->version < Version(93, 0) && parts[i].ctype == SPC_AIR)
						|| (save->version < Version(88, 0) && parts[i].ctype == OLD_SPC_AIR))
				{
					fan = true;
					parts[i].ctype = 0;
				}
				fighters[parts[i].tmp].elem = PT_DUST;
				Element_FIGH_NewFighter(this, parts[i].tmp, i, parts[i].ctype);
				if (fan)
					fighters[parts[i].tmp].fan = true;
				for (unsigned int fighNum : save->stkm.rocketBootsFigh)
				{
					if (fighNum == oldTmp)
						fighters[parts[i].tmp].rocketBoots = true;
				}
				for (unsigned int fighNum : save->stkm.fanFigh)
				{
					if (fighNum == oldTmp)
						fighters[parts[i].tmp].fan = true;
				}
			}
			else
			{
				// Should not be possible because we verify with CanAlloc above this
				parts[i].type = 0;
			}
			break;
		}
		case PT_SOAP:
			soapList.insert(std::pair<unsigned int, unsigned int>(n, i));
			break;
		}
		if (GameSave::PressureInTmp3(parts[i].type) && !includePressure)
		{
			parts[i].tmp3 = 0;
		}
	}
	parts_lastActiveIndex = NPART-1;
	force_stacking_check = true;
	Element_PPIP_ppip_changed = 1;

	// Sort out pmap, just to be on the safe side.
	RecalcFreeParticles(false);

	// fix SOAP links using soapList, a map of old particle ID -> new particle ID
	// loop through every old particle (loaded from save), and convert .tmp / .tmp2
	for (std::map<unsigned int, unsigned int>::iterator iter = soapList.begin(), end = soapList.end(); iter != end; ++iter)
	{
		int i = (*iter).second;
		if ((parts[i].ctype & 0x2) == 2)
		{
			std::map<unsigned int, unsigned int>::iterator n = soapList.find(parts[i].tmp);
			if (n != end)
				parts[i].tmp = n->second;
			// sometimes the proper SOAP isn't found. It should remove the link, but seems to break some saves
			// so just ignore it
		}
		if ((parts[i].ctype & 0x4) == 4)
		{
			std::map<unsigned int, unsigned int>::iterator n = soapList.find(parts[i].tmp2);
			if (n != end)
				parts[i].tmp2 = n->second;
			// sometimes the proper SOAP isn't found. It should remove the link, but seems to break some saves
			// so just ignore it
		}
	}

	for (size_t i = 0; i < save->signs.size() && signs.size() < MAXSIGNS; i++)
	{
		if (save->signs[i].text.length())
		{
			sign tempSign = save->signs[i];
			tempSign.x += partP.X;
			tempSign.y += partP.Y;
			if (!InBounds(tempSign.x, tempSign.y))
			{
				continue;
			}
			signs.push_back(tempSign);
		}
	}
	for (auto bpos : RectSized(blockP, save->blockSize) & CELLS.OriginRect())
	{
		auto spos = bpos - blockP;
		if (save->blockMap[spos])
		{
			bmap[bpos.Y][bpos.X] = save->blockMap[spos];
			fvx[bpos.Y][bpos.X] = save->fanVelX[spos];
			fvy[bpos.Y][bpos.X] = save->fanVelY[spos];
		}
		if (includePressure)
		{
			if (save->hasPressure)
			{
				pv[bpos.Y][bpos.X] = save->pressure[spos];
				vx[bpos.Y][bpos.X] = save->velocityX[spos];
				vy[bpos.Y][bpos.X] = save->velocityY[spos];
			}
			if (save->hasAmbientHeat)
			{
				hv[bpos.Y][bpos.X] = save->ambientHeat[spos];
			}
			if (save->hasBlockAirMaps)
			{
				air->bmap_blockair[bpos.Y][bpos.X] = save->blockAir[spos];
				air->bmap_blockairh[bpos.Y][bpos.X] = save->blockAirh[spos];
			}
		}
	}

	gravWallChanged = true;
	if (!save->hasBlockAirMaps)
	{
		air->ApproximateBlockAirMaps();
	}
}

std::unique_ptr<GameSave> Simulation::Save(bool includePressure, Rect<int> partR) // particle coordinates
{
	auto blockR = RectBetween(partR.TopLeft / CELL, partR.BottomRight / CELL);
	auto blockP = blockR.TopLeft;

	auto newSave = std::make_unique<GameSave>(blockR.Size());
	newSave->frameCount = frameCount;
	newSave->rngState = rng.state();

	int storedParts = 0;
	int elementCount[PT_NUM];
	std::fill(elementCount, elementCount+PT_NUM, 0);
	// Map of soap particles loaded into this save, old ID -> new ID
	// Now stores all particles, not just SOAP (but still only used for soap)
	std::map<unsigned int, unsigned int> particleMap;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	for (int i = 0; i < NPART; i++)
	{
		int x, y;
		x = int(parts[i].x + 0.5f);
		y = int(parts[i].y + 0.5f);
		if (parts[i].type && partR.Contains({ x, y }))
		{
			Particle tempPart = parts[i];
			tempPart.x -= blockP.X * CELL;
			tempPart.y -= blockP.Y * CELL;
			if (elements[tempPart.type].Enabled)
			{
				particleMap.insert(std::pair<unsigned int, unsigned int>(i, storedParts));
				*newSave << tempPart;
				storedParts++;
				elementCount[tempPart.type]++;

			}
		}
	}

	if (storedParts && elementCount[PT_SOAP])
	{
		// fix SOAP links using particleMap, a map of old particle ID -> new particle ID
		// loop through every new particle (saved into the save), and convert .tmp / .tmp2
		for (std::map<unsigned int, unsigned int>::iterator iter = particleMap.begin(), end = particleMap.end(); iter != end; ++iter)
		{
			int i = (*iter).second;
			if (newSave->particles[i].type != PT_SOAP)
				continue;
			if ((newSave->particles[i].ctype & 0x2) == 2)
			{
				std::map<unsigned int, unsigned int>::iterator n = particleMap.find(newSave->particles[i].tmp);
				if (n != end)
					newSave->particles[i].tmp = n->second;
				else
				{
					newSave->particles[i].tmp = 0;
					newSave->particles[i].ctype ^= 2;
				}
			}
			if ((newSave->particles[i].ctype & 0x4) == 4)
			{
				std::map<unsigned int, unsigned int>::iterator n = particleMap.find(newSave->particles[i].tmp2);
				if (n != end)
					newSave->particles[i].tmp2 = n->second;
				else
				{
					newSave->particles[i].tmp2 = 0;
					newSave->particles[i].ctype ^= 4;
				}
			}
		}
	}

	for (size_t i = 0; i < MAXSIGNS && i < signs.size(); i++)
	{
		if (signs[i].text.length() && partR.Contains({ signs[i].x, signs[i].y }))
		{
			sign tempSign = signs[i];
			tempSign.x -= blockP.X * CELL;
			tempSign.y -= blockP.Y * CELL;
			*newSave << tempSign;
		}
	}

	for (auto bpos : newSave->blockSize.OriginRect())
	{
		if(bmap[bpos.Y + blockP.Y][bpos.X + blockP.X])
		{
			newSave->blockMap[bpos] = bmap[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->fanVelX[bpos] = fvx[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->fanVelY[bpos] = fvy[bpos.Y + blockP.Y][bpos.X + blockP.X];
		}
		if (includePressure)
		{
			newSave->pressure[bpos] = pv[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->velocityX[bpos] = vx[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->velocityY[bpos] = vy[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->ambientHeat[bpos] = hv[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->blockAir[bpos] = air->bmap_blockair[bpos.Y + blockP.Y][bpos.X + blockP.X];
			newSave->blockAirh[bpos] = air->bmap_blockairh[bpos.Y + blockP.Y][bpos.X + blockP.X];
		}
	}
	if (includePressure || ensureDeterminism)
	{
		newSave->hasPressure = true;
		newSave->hasAmbientHeat = true;
	}
	newSave->ensureDeterminism = ensureDeterminism;

	newSave->stkm.rocketBoots1 = player.rocketBoots;
	newSave->stkm.rocketBoots2 = player2.rocketBoots;
	newSave->stkm.fan1 = player.fan;
	newSave->stkm.fan2 = player2.fan;
	for (unsigned char i = 0; i < MAX_FIGHTERS; i++)
	{
		if (fighters[i].rocketBoots)
			newSave->stkm.rocketBootsFigh.push_back(i);
		if (fighters[i].fan)
			newSave->stkm.fanFigh.push_back(i);
	}

	SaveSimOptions(*newSave);
	newSave->pmapbits = PMAPBITS;
	return newSave;
}

void Simulation::SaveSimOptions(GameSave &gameSave)
{
	gameSave.gravityMode = gravityMode;
	gameSave.customGravityX = customGravityX;
	gameSave.customGravityY = customGravityY;
	gameSave.airMode = air->airMode;
	gameSave.ambientAirTemp = air->ambientAirTemp;
	gameSave.edgeMode = edgeMode;
	gameSave.legacyEnable = legacy_enable;
	gameSave.waterEEnabled = water_equal_test;
	gameSave.gravityEnable = grav->IsEnabled();
	gameSave.aheatEnable = aheat_enable;
}

bool Simulation::FloodFillPmapCheck(int x, int y, int type) const
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (type == 0)
		return !pmap[y][x] && !photons[y][x];
	if (elements[type].Properties&TYPE_ENERGY)
		return TYP(photons[y][x]) == type;
	else
		return TYP(pmap[y][x]) == type;
}

CoordStack& Simulation::getCoordStackSingleton()
{
	// Future-proofing in case Simulation is later multithreaded
	static THREAD_LOCAL(CoordStack, cs);
	return cs;
}

int Simulation::flood_prop(int x, int y, StructProperty prop, PropertyValue propvalue)
{
	int i, x1, x2, dy = 1;
	int did_something = 0;
	int r = pmap[y][x];
	if (!r)
		r = photons[y][x];
	if (!r)
		return 0;
	int parttype = TYP(r);
	char * bitmap = (char*)malloc(XRES*YRES); //Bitmap for checking
	if (!bitmap) return -1;
	memset(bitmap, 0, XRES*YRES);
	try
	{
		CoordStack& cs = getCoordStackSingleton();
		cs.clear();

		cs.push(x, y);
		do
		{
			cs.pop(x, y);
			x1 = x2 = x;
			while (x1>=CELL)
			{
				if (!FloodFillPmapCheck(x1-1, y, parttype) || bitmap[(y*XRES)+x1-1])
					break;
				x1--;
			}
			while (x2<XRES-CELL)
			{
				if (!FloodFillPmapCheck(x2+1, y, parttype) || bitmap[(y*XRES)+x2+1])
					break;
				x2++;
			}
			for (x=x1; x<=x2; x++)
			{
				i = pmap[y][x];
				if (!i)
					i = photons[y][x];
				if (!i)
					continue;
				switch (prop.Type) {
					case StructProperty::Float:
						*((float*)(((char*)&parts[ID(i)])+prop.Offset)) = std::get<float>(propvalue);
						break;

					case StructProperty::ParticleType:
					case StructProperty::Integer:
						*((int*)(((char*)&parts[ID(i)])+prop.Offset)) = std::get<int>(propvalue);
						break;

					case StructProperty::UInteger:
						*((unsigned int*)(((char*)&parts[ID(i)])+prop.Offset)) = std::get<unsigned int>(propvalue);
						break;

					default:
						break;
				}
				bitmap[(y*XRES)+x] = 1;
				did_something = 1;
			}
			if (y>=CELL+dy)
				for (x=x1; x<=x2; x++)
					if (FloodFillPmapCheck(x, y-dy, parttype) && !bitmap[((y-dy)*XRES)+x])
						cs.push(x, y-dy);
			if (y<YRES-CELL-dy)
				for (x=x1; x<=x2; x++)
					if (FloodFillPmapCheck(x, y+dy, parttype) && !bitmap[((y+dy)*XRES)+x])
						cs.push(x, y+dy);
		} while (cs.getSize()>0);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		free(bitmap);
		return -1;
	}
	free(bitmap);
	return did_something;
}

int Simulation::FloodINST(int x, int y)
{
	int x1, x2;
	int created_something = 0;

	const auto isSparkableInst = [this](int x, int y) -> bool {
		return TYP(pmap[y][x])==PT_INST && parts[ID(pmap[y][x])].life==0;
	};

	const auto isInst = [this](int x, int y) -> bool {
		return TYP(pmap[y][x])==PT_INST || (TYP(pmap[y][x])==PT_SPRK  && parts[ID(pmap[y][x])].ctype==PT_INST);
	};

	if (!isSparkableInst(x,y))
		return 1;

	CoordStack& cs = getCoordStackSingleton();
	cs.clear();

	cs.push(x, y);

	try
	{
		do
		{
			cs.pop(x, y);
			x1 = x2 = x;
			// go left as far as possible
			while (x1>=CELL && isSparkableInst(x1-1, y))
			{
				x1--;
			}
			// go right as far as possible
			while (x2<XRES-CELL && isSparkableInst(x2+1, y))
			{
				x2++;
			}
			// fill span
			for (x=x1; x<=x2; x++)
			{
				if (create_part(-1, x, y, PT_SPRK)>=0)
					created_something = 1;
			}

			// add vertically adjacent pixels to stack
			// (wire crossing for INST)
			if (y>=CELL+1 && x1==x2 &&
				isInst(x1-1, y-1) && isInst(x1, y-1) && isInst(x1+1, y-1) &&
				!isInst(x1-1, y-2) && isInst(x1, y-2) && !isInst(x1+1, y-2))
			{
				// travelling vertically up, skipping a horizontal line
				if (isSparkableInst(x1, y-2))
				{
						cs.push(x1, y-2);
				}
			}
			else if (y>=CELL+1)
			{
				for (x=x1; x<=x2; x++)
				{
					if (isSparkableInst(x, y-1))
					{
						if (x==x1 || x==x2 || y>=YRES-CELL-1 || !isInst(x, y+1) || isInst(x+1, y+1) || isInst(x-1, y+1))
						{
							// if at the end of a horizontal section, or if it's a T junction or not a 1px wire crossing
							cs.push(x, y-1);
						}
					}
				}
			}

			if (y<YRES-CELL-1 && x1==x2 &&
				isInst(x1-1, y+1) && isInst(x1, y+1) && isInst(x1+1, y+1) &&
				!isInst(x1-1, y+2) && isInst(x1, y+2) && !isInst(x1+1, y+2))
			{
				// travelling vertically down, skipping a horizontal line
				if (isSparkableInst(x1, y+2))
				{
					cs.push(x1, y+2);
				}
			}
			else if (y<YRES-CELL-1)
			{
				for (x=x1; x<=x2; x++)
				{
					if (isSparkableInst(x, y+1))
					{
						if (x==x1 || x==x2 || y<0 || !isInst(x, y-1) || isInst(x+1, y-1) || isInst(x-1, y-1))
						{
							// if at the end of a horizontal section, or if it's a T junction or not a 1px wire crossing
							cs.push(x, y+1);
						}

					}
				}
			}
		} while (cs.getSize()>0);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return created_something;
}

bool Simulation::flood_water(int x, int y, int i)
{
	int x1, x2, originalX = x, originalY = y;
	int r = pmap[y][x];
	if (!r)
		return false;

	// Bitmap for checking where we've already looked
	auto bitmapPtr = std::unique_ptr<char[]>(new char[XRES * YRES]);
	char *bitmap = bitmapPtr.get();
	std::fill(&bitmap[0], &bitmap[0] + XRES * YRES, 0);

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	try
	{
		CoordStack& cs = getCoordStackSingleton();
		cs.clear();

		cs.push(x, y);
		do
		{
			cs.pop(x, y);
			x1 = x2 = x;
			while (x1 >= CELL)
			{
				if (elements[TYP(pmap[y][x1 - 1])].Falldown != 2 || bitmap[(y * XRES) + x1 - 1])
					break;
				x1--;
			}
			while (x2 < XRES-CELL)
			{
				if (elements[TYP(pmap[y][x2 + 1])].Falldown != 2 || bitmap[(y * XRES) + x1 - 1])
					break;
				x2++;
			}
			for (int x = x1; x <= x2; x++)
			{
				if ((y - 1) > originalY && !pmap[y - 1][x])
				{
					// Try to move the water to a random position on this line, because there's probably a free location somewhere
					int randPos = rng.between(x, x2);
					if (!pmap[y - 1][randPos] && eval_move(parts[i].type, randPos, y - 1, nullptr))
						x = randPos;
					// Couldn't move to random position, so try the original position on the left
					else if (!eval_move(parts[i].type, x, y - 1, nullptr))
						continue;

					move(i, originalX, originalY, float(x), float(y - 1));
					return true;
				}

				bitmap[(y * XRES) + x] = 1;
			}
			if (y >= CELL + 1)
				for (int x = x1; x <= x2; x++)
					if (elements[TYP(pmap[y - 1][x])].Falldown == 2 && !bitmap[((y - 1) * XRES) + x])
						cs.push(x, y - 1);
			if (y < YRES - CELL - 1)
				for (int x = x1; x <= x2; x++)
					if (elements[TYP(pmap[y + 1][x])].Falldown == 2 && !bitmap[((y + 1) * XRES) + x])
						cs.push(x, y + 1);
		} while (cs.getSize() > 0);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
	return false;
}

void Simulation::SetEdgeMode(int newEdgeMode)
{
	edgeMode = newEdgeMode;
	switch(edgeMode)
	{
	case EDGE_VOID:
	case EDGE_LOOP:
		for(int i = 0; i<XCELLS; i++)
		{
			bmap[0][i] = 0;
			bmap[YCELLS-1][i] = 0;
		}
		for(int i = 1; i<(YCELLS-1); i++)
		{
			bmap[i][0] = 0;
			bmap[i][XCELLS-1] = 0;
		}
		break;
	case EDGE_SOLID:
		int i;
		for(i=0; i<XCELLS; i++)
		{
			bmap[0][i] = WL_WALL;
			bmap[YCELLS-1][i] = WL_WALL;
		}
		for(i=1; i<(YCELLS-1); i++)
		{
			bmap[i][0] = WL_WALL;
			bmap[i][XCELLS-1] = WL_WALL;
		}
		break;
	default:
		SetEdgeMode(EDGE_VOID);
	}
}

// Now simply creates a 0 pixel radius line without all the complicated flags / other checks
// Would make sense to move to Editing.cpp but SPRK needs it.
void Simulation::CreateLine(int x1, int y1, int x2, int y2, int c)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy;
	float e, de;
	int v = ID(c);
	c = TYP(c);
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	e = 0.0f;
	de = dx ? dy/(float)dx : 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			create_part(-1, y, x, c, v);
		else
			create_part(-1, x, y, c, v);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if ((y1<y2) ? (y<=y2) : (y>=y2))
			{
				if (reverseXY)
					create_part(-1, y, x, c, v);
				else
					create_part(-1, x, y, c, v);
			}
			e -= 1.0f;
		}
	}
}

inline int Simulation::is_wire(int x, int y)
{
	return bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE || bmap[y][x]==WL_STASIS;
}

inline int Simulation::is_wire_off(int x, int y)
{
	return (bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE || bmap[y][x]==WL_STASIS) && emap[y][x]<8;
}

// implement __builtin_ctz and __builtin_clz on msvc
#ifdef _MSC_VER
unsigned msvc_ctz(unsigned a)
{
	unsigned long i;
	_BitScanForward(&i, a);
	return i;
}

unsigned msvc_clz(unsigned a)
{
	unsigned long i;
	_BitScanReverse(&i, a);
	return 31 - i;
}

#define __builtin_ctz msvc_ctz
#define __builtin_clz msvc_clz
#endif

int Simulation::get_wavelength_bin(int *wm)
{
	int i, w0, wM, r;

	if (!(*wm & 0x3FFFFFFF))
		return -1;

#if defined(__GNUC__) || defined(_MSVC_VER)
	w0 = __builtin_ctz(*wm | 0xC0000000);
	wM = 31 - __builtin_clz(*wm & 0x3FFFFFFF);
#else
	w0 = 30;
	wM = 0;
	for (i = 0; i < 30; i++)
		if (*wm & (1<<i))
		{
			if (i < w0)
				w0 = i;
			if (i > wM)
				wM = i;
		}
#endif

	if (wM - w0 < 5)
		return wM + w0;

	r = rng.gen();
	i = (r >> 1) % (wM-w0-4);
	i += w0;

	if (r & 1)
	{
		*wm &= 0x1F << i;
		return (i + 2) * 2;
	}
	else
	{
		*wm &= 0xF << i;
		return (i + 2) * 2 - 1;
	}
}

void Simulation::set_emap(int x, int y)
{
	int x1, x2;

	if (!is_wire_off(x, y))
		return;

	// go left as far as possible
	x1 = x2 = x;
	while (x1>0)
	{
		if (!is_wire_off(x1-1, y))
			break;
		x1--;
	}
	while (x2<XCELLS-1)
	{
		if (!is_wire_off(x2+1, y))
			break;
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
		emap[y][x] = 16;

	// fill children

	if (y>1 && x1==x2 &&
	        is_wire(x1-1, y-1) && is_wire(x1, y-1) && is_wire(x1+1, y-1) &&
	        !is_wire(x1-1, y-2) && is_wire(x1, y-2) && !is_wire(x1+1, y-2))
		set_emap(x1, y-2);
	else if (y>0)
		for (x=x1; x<=x2; x++)
			if (is_wire_off(x, y-1))
			{
				if (x==x1 || x==x2 || y>=YCELLS-1 ||
				        is_wire(x-1, y-1) || is_wire(x+1, y-1) ||
				        is_wire(x-1, y+1) || !is_wire(x, y+1) || is_wire(x+1, y+1))
					set_emap(x, y-1);
			}

	if (y<YCELLS-2 && x1==x2 &&
	        is_wire(x1-1, y+1) && is_wire(x1, y+1) && is_wire(x1+1, y+1) &&
	        !is_wire(x1-1, y+2) && is_wire(x1, y+2) && !is_wire(x1+1, y+2))
		set_emap(x1, y+2);
	else if (y<YCELLS-1)
		for (x=x1; x<=x2; x++)
			if (is_wire_off(x, y+1))
			{
				if (x==x1 || x==x2 || y<0 ||
				        is_wire(x-1, y+1) || is_wire(x+1, y+1) ||
				        is_wire(x-1, y-1) || !is_wire(x, y-1) || is_wire(x+1, y-1))
					set_emap(x, y+1);
			}
}

int Simulation::parts_avg(int ci, int ni,int t)
{
	if (t==PT_INSL)//to keep electronics working
	{
		int pmr = pmap[((int)(parts[ci].y+0.5f) + (int)(parts[ni].y+0.5f))/2][((int)(parts[ci].x+0.5f) + (int)(parts[ni].x+0.5f))/2];
		if (pmr)
			return parts[ID(pmr)].type;
		else
			return PT_NONE;
	}
	else
	{
		int pmr2 = pmap[(int)((parts[ci].y + parts[ni].y)/2+0.5f)][(int)((parts[ci].x + parts[ni].x)/2+0.5f)];//seems to be more accurate.
		if (pmr2)
		{
			if (parts[ID(pmr2)].type==t)
				return t;
		}
		else
			return PT_NONE;
	}
	return PT_NONE;
}

void Simulation::clear_sim(void)
{
	ensureDeterminism = false;
	frameCount = 0;
	debug_nextToUpdate = 0;
	debug_mostRecentlyUpdated = -1;
	emp_decor = 0;
	emp_trigger_count = 0;
	signs.clear();
	memset(bmap, 0, sizeof(bmap));
	memset(emap, 0, sizeof(emap));
	memset(parts, 0, sizeof(Particle)*NPART);
	for (int i = 0; i < NPART-1; i++)
		parts[i].life = i+1;
	parts[NPART-1].life = -1;
	pfree = 0;
	parts_lastActiveIndex = 0;
	memset(pmap, 0, sizeof(pmap));
	memset(fvx, 0, sizeof(fvx));
	memset(fvy, 0, sizeof(fvy));
	memset(photons, 0, sizeof(photons));
	memset(wireless, 0, sizeof(wireless));
	memset(gol, 0, sizeof(gol));
	memset(portalp, 0, sizeof(portalp));
	memset(fighters, 0, sizeof(fighters));
	memset(&player, 0, sizeof(player));
	memset(&player2, 0, sizeof(player2));
	std::fill(elementCount, elementCount+PT_NUM, 0);
	elementRecount = true;
	fighcount = 0;
	player.spwn = 0;
	player.spawnID = -1;
	player.rocketBoots = false;
	player.fan = false;
	player2.spwn = 0;
	player2.spawnID = -1;
	player2.rocketBoots = false;
	player2.fan = false;
	//memset(pers_bg, 0, WINDOWW*YRES*PIXELSIZE);
	//memset(fire_r, 0, sizeof(fire_r));
	//memset(fire_g, 0, sizeof(fire_g));
	//memset(fire_b, 0, sizeof(fire_b));
	//if(gravmask)
		//memset(gravmask, 0xFFFFFFFF, NCELL*sizeof(unsigned));
	if(grav)
		grav->Clear();
	if(air)
	{
		air->Clear();
		air->ClearAirH();
	}
	SetEdgeMode(edgeMode);
}

bool Simulation::IsWallBlocking(int x, int y, int type) const
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (bmap[y/CELL][x/CELL])
	{
		int wall = bmap[y/CELL][x/CELL];
		if (wall == WL_ALLOWGAS && !(elements[type].Properties&TYPE_GAS))
			return true;
		else if (wall == WL_ALLOWENERGY && !(elements[type].Properties&TYPE_ENERGY))
			return true;
		else if (wall == WL_ALLOWLIQUID && !(elements[type].Properties&TYPE_LIQUID))
			return true;
		else if (wall == WL_ALLOWPOWDER && !(elements[type].Properties&TYPE_PART))
			return true;
		else if (wall == WL_ALLOWAIR || wall == WL_WALL || wall == WL_WALLELEC)
			return true;
		else if (wall == WL_EWALL && !emap[y/CELL][x/CELL])
			return true;
		else if (wall == WL_DETECT && (elements[type].Properties&TYPE_SOLID))
			return true;
	}
	return false;
}

/*
   RETURN-value explanation
1 = Swap
0 = No move/Bounce
2 = Both particles occupy the same space.
 */
int Simulation::eval_move(int pt, int nx, int ny, unsigned *rr) const
{
	unsigned r;
	int result;

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 0;

	r = pmap[ny][nx];
	if (r)
		r = (r&~PMAPMASK) | parts[ID(r)].type;
	if (rr)
		*rr = r;
	if (pt>=PT_NUM || TYP(r)>=PT_NUM)
		return 0;
	auto &sd = SimulationData::CRef();
	auto &can_move = sd.can_move;
	auto &elements = sd.elements;
	result = can_move[pt][TYP(r)];
	if (result == 3)
	{
		switch (TYP(r))
		{
		case PT_LCRY:
			if (pt==PT_PHOT)
				result = (parts[ID(r)].life > 5)? 2 : 0;
			break;
		case PT_GPMP:
			if (pt == PT_PHOT)
				result = (parts[ID(r)].life < 10) ? 2 : 0;
			break;
		case PT_INVIS:
		{
			float pressureResistance = 0.0f;
			if (parts[ID(r)].tmp > 0)
				pressureResistance = (float)parts[ID(r)].tmp;
			else
				pressureResistance = 4.0f;

			if (pv[ny/CELL][nx/CELL] < -pressureResistance || pv[ny/CELL][nx/CELL] > pressureResistance)
				result = 2;
			else
				result = 0;
			break;
		}
		case PT_PVOD:
			if (parts[ID(r)].life == 10)
			{
				if (!parts[ID(r)].ctype || (parts[ID(r)].ctype==pt)!=(parts[ID(r)].tmp&1))
					result = 1;
				else
					result = 0;
			}
			else result = 0;
			break;
		case PT_VOID:
			if (!parts[ID(r)].ctype || (parts[ID(r)].ctype==pt)!=(parts[ID(r)].tmp&1))
				result = 1;
			else
				result = 0;
			break;
		case PT_SWCH:
			if (pt == PT_TRON)
			{
				if (parts[ID(r)].life >= 10)
					return 2;
				else
					return 0;
			}
			break;
		default:
			// This should never happen
			// If it were to happen, try_move would interpret a 3 as a 1
			result =  1;
		}
	}
	if (bmap[ny/CELL][nx/CELL])
	{
		if (IsWallBlocking(nx, ny, pt))
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL] && !(elements[pt].Properties&TYPE_SOLID) && !(elements[TYP(r)].Properties&TYPE_SOLID))
			return 2;
	}
	return result;
}

int Simulation::try_move(int i, int x, int y, int nx, int ny)
{
	unsigned r = 0, e;

	if (x==nx && y==ny)
		return 1;
	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 1;

	e = eval_move(parts[i].type, nx, ny, &r);

	/* half-silvered mirror */
	if (!e && parts[i].type==PT_PHOT && ((TYP(r)==PT_BMTL && rng.chance(1, 2)) || TYP(pmap[y][x])==PT_BMTL))
		e = 2;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (!e) //if no movement
	{
		int rt = TYP(r);
		if (rt == PT_WOOD)
		{
			float vel = std::sqrt(std::pow(parts[i].vx, 2) + std::pow(parts[i].vy, 2));
			if (vel > 5)
				part_change_type(ID(r), nx, ny, PT_SAWD);
		}
		if (!(elements[parts[i].type].Properties & TYPE_ENERGY))
			return 0;
		if (!legacy_enable && parts[i].type==PT_PHOT && r)//PHOT heat conduction
		{
			if (rt == PT_COAL || rt == PT_BCOL)
				parts[ID(r)].temp = parts[i].temp;

			if (rt < PT_NUM && elements[rt].HeatConduct && (rt!=PT_HSWC||parts[ID(r)].life==10) && rt!=PT_FILT)
				parts[i].temp = parts[ID(r)].temp = restrict_flt((parts[ID(r)].temp+parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
		}
		else if ((parts[i].type==PT_NEUT || parts[i].type==PT_ELEC) && (rt==PT_CLNE || rt==PT_PCLN || rt==PT_BCLN || rt==PT_PBCN))
		{
			if (!parts[ID(r)].ctype)
				parts[ID(r)].ctype = parts[i].type;
		}
		if (rt==PT_PRTI && (elements[parts[i].type].Properties & TYPE_ENERGY))
		{
			int nnx, count;
			for (count=0; count<8; count++)
			{
				if (isign(x-nx)==isign(portal_rx[count]) && isign(y-ny)==isign(portal_ry[count]))
					break;
			}
			count = count%8;
			parts[ID(r)].tmp = (int)((parts[ID(r)].temp-73.15f)/100+1);
			if (parts[ID(r)].tmp>=CHANNELS) parts[ID(r)].tmp = CHANNELS-1;
			else if (parts[ID(r)].tmp<0) parts[ID(r)].tmp = 0;
			for ( nnx=0; nnx<80; nnx++)
				if (!portalp[parts[ID(r)].tmp][count][nnx].type)
				{
					portalp[parts[ID(r)].tmp][count][nnx] = parts[i];
					parts[i].type=PT_NONE;
					break;
				}
		}
		return 0;
	}

	if (e == 2) //if occupy same space
	{
		switch (parts[i].type)
		{
		case PT_PHOT:
		{
			switch (TYP(r))
			{
			case PT_GLOW:
				if (!parts[ID(r)].life && rng.chance(29, 30))
				{
					parts[ID(r)].life = 120;
					create_gain_photon(i);
				}
				break;
			case PT_FILT:
				parts[i].ctype = Element_FILT_interactWavelengths(this, &parts[ID(r)], parts[i].ctype);
				break;
			case PT_C5:
				if (parts[ID(r)].life > 0 && (parts[ID(r)].ctype & parts[i].ctype & 0xFFFFFFC0))
				{
					float vx = ((parts[ID(r)].tmp << 16) >> 16) / 255.0f;
					float vy = (parts[ID(r)].tmp >> 16) / 255.0f;
					float vn = parts[i].vx * parts[i].vx + parts[i].vy * parts[i].vy;
					// if the resulting velocity would be 0, that would cause division by 0 inside the else
					// shoot the photon off at a 90 degree angle instead (probably particle order dependent)
					if (parts[i].vx + vx == 0 && parts[i].vy + vy == 0)
					{
						parts[i].vx = vy;
						parts[i].vy = -vx;
					}
					else
					{
						parts[i].ctype = (parts[ID(r)].ctype & parts[i].ctype) >> 6;
						// add momentum of photons to each other
						parts[i].vx += vx;
						parts[i].vy += vy;
						// normalize velocity to original value
						vn /= parts[i].vx * parts[i].vx + parts[i].vy * parts[i].vy;
						vn = sqrtf(vn);
						parts[i].vx *= vn;
						parts[i].vy *= vn;
					}
					parts[ID(r)].life = 0;
					parts[ID(r)].ctype = 0;
				}
				else if(!parts[ID(r)].ctype && parts[i].ctype & 0xFFFFFFC0)
				{
					parts[ID(r)].life = 1;
					parts[ID(r)].ctype = parts[i].ctype;
					parts[ID(r)].tmp = (0xFFFF & (int)(parts[i].vx * 255.0f)) | (0xFFFF0000 & (int)(parts[i].vy * 16711680.0f));
					parts[ID(r)].tmp2 = (0xFFFF & (int)((parts[i].x - x) * 255.0f)) | (0xFFFF0000 & (int)((parts[i].y - y) * 16711680.0f));
					kill_part(i);
				}
				break;
			case PT_INVIS:
			{
				float pressureResistance = 0.0f;
				pressureResistance = (parts[ID(r)].tmp > 0) ? (float)parts[ID(r)].tmp : 4.0f;

				if (pv[ny/CELL][nx/CELL] >= -pressureResistance && pv[ny/CELL][nx/CELL] <= pressureResistance)
				{
					part_change_type(i,x,y,PT_NEUT);
					parts[i].ctype = 0;
				}
				break;
			}
			case PT_BIZR:
			case PT_BIZRG:
			case PT_BIZRS:
				part_change_type(i, x, y, PT_ELEC);
				parts[i].ctype = 0;
				break;
			case PT_H2:
				if (!(parts[i].tmp&0x1))
				{
					part_change_type(i, x, y, PT_PROT);
					parts[i].ctype = 0;
					parts[i].tmp2 = 0x1;

					create_part(ID(r), x, y, PT_ELEC);
					return 1;
				}
				break;
			case PT_GPMP:
				if (parts[ID(r)].life == 0)
				{
					part_change_type(i, x, y, PT_GRVT);
					parts[i].tmp = int(parts[ID(r)].temp - 273.15f);
				}
				break;
			}
			break;
		}
		case PT_NEUT:
			if (TYP(r) == PT_GLAS || TYP(r) == PT_BGLA)
				if (rng.chance(9, 10))
					create_cherenkov_photon(i);
			break;
		case PT_ELEC:
			if (TYP(r) == PT_GLOW)
			{
				part_change_type(i, x, y, PT_PHOT);
				parts[i].ctype = 0x3FFFFFFF;
			}
			break;
		case PT_PROT:
			if (TYP(r) == PT_INVIS)
				part_change_type(i, x, y, PT_NEUT);
			break;
		case PT_BIZR:
		case PT_BIZRG:
			if (TYP(r) == PT_FILT)
				parts[i].ctype = Element_FILT_interactWavelengths(this, &parts[ID(r)], parts[i].ctype);
			break;
		}
		return 1;
	}
	//else e=1 , we are trying to swap the particles, return 0 no swap/move, 1 is still overlap/move, because the swap takes place later

	switch (TYP(r))
	{
	case PT_VOID:
	case PT_PVOD:
		// this is where void eats particles
		// void ctype already checked in eval_move
		kill_part(i);
		return 0;
	case PT_BHOL:
	case PT_NBHL:
		// this is where blackhole eats particles
		if (!legacy_enable)
		{
			parts[ID(r)].temp = restrict_flt(parts[ID(r)].temp+parts[i].temp/2, MIN_TEMP, MAX_TEMP);//3.0f;
		}
		kill_part(i);
		return 0;
	case PT_WHOL:
	case PT_NWHL:
		// whitehole eats anar
		if (parts[i].type == PT_ANAR)
		{
			if (!legacy_enable)
			{
				parts[ID(r)].temp = restrict_flt(parts[ID(r)].temp - (MAX_TEMP-parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
			}
			kill_part(i);
			return 0;
		}
		break;
	case PT_DEUT:
		if (parts[i].type == PT_ELEC)
		{
			if(parts[ID(r)].life < 6000)
				parts[ID(r)].life += 1;
			parts[ID(r)].temp = 0;
			kill_part(i);
			return 0;
		}
		break;
	case PT_VIBR:
	case PT_BVBR:
		if ((elements[parts[i].type].Properties & TYPE_ENERGY))
		{
			parts[ID(r)].tmp += 20;
			kill_part(i);
			return 0;
		}
		break;
	}

	switch (parts[i].type)
	{
	case PT_NEUT:
		if (elements[TYP(r)].Properties & PROP_NEUTABSORB)
		{
			kill_part(i);
			return 0;
		}
		break;
	case PT_CNCT:
		{
			float cnctGravX, cnctGravY; // Calculate offset from gravity
			GetGravityField(x, y, elements[PT_CNCT].Gravity, elements[PT_CNCT].Gravity, cnctGravX, cnctGravY);
			int offsetX = 0, offsetY = 0;
			if (cnctGravX > 0.0f) offsetX++;
			else if (cnctGravX < 0.0f) offsetX--;
			if (cnctGravY > 0.0f) offsetY++;
			else if (cnctGravY < 0.0f) offsetY--;
			if ((offsetX != 0) != (offsetY != 0) && // Is this a different position (avoid diagonals, doesn't work well)
				((nx - x) * offsetX > 0 || (ny - y) * offsetY > 0) && // Is the destination particle below the moving particle
				(TYP(pmap[y+offsetY][x+offsetX]) == PT_CNCT || TYP(pmap[y+offsetY][x+offsetX]) == PT_ROCK)) //check below CNCT for another CNCT or ROCK
				return 0;
		}
		break;
	case PT_GBMB:
		if (parts[i].life > 0)
			return 0;
		break;
	}

	if ((bmap[y/CELL][x/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL]) && !(bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL]))
		return 0;

	int ri = ID(r); //ri is the particle number at r (pmap[ny][nx])
	if (r)//the swap part, if we make it this far, swap
	{
		if (parts[i].type==PT_NEUT) {
			// target material is NEUTPENETRATE, meaning it gets moved around when neutron passes
			unsigned s = pmap[y][x];
			if (s && !(elements[TYP(s)].Properties&PROP_NEUTPENETRATE))
				return 1; // if the element currently underneath neutron isn't NEUTPENETRATE, don't move anything except the neutron
			// if nothing is currently underneath neutron, only move target particle
			if(bmap[y/CELL][x/CELL] == WL_ALLOWENERGY)
				return 1; // do not drag target particle into an energy only wall
			if (s)
			{
				pmap[ny][nx] = (s&~PMAPMASK)|parts[ID(s)].type;
				parts[ID(s)].x = float(nx);
				parts[ID(s)].y = float(ny);
			}
			else
				pmap[ny][nx] = 0;
			parts[ri].x = float(x);
			parts[ri].y = float(y);
			pmap[y][x] = PMAP(ri, parts[ri].type);
			return 1;
		}

		if (pmap[ny][nx] && ID(pmap[ny][nx]) == ri)
			pmap[ny][nx] = 0;
		parts[ri].x += float(x - nx);
		parts[ri].y += float(y - ny);
		int rx = int(parts[ri].x + 0.5f);
		int ry = int(parts[ri].y + 0.5f);
		// This check will never fail unless the pmap array has already been corrupted via another bug
		// In that case, r's position is inaccurate (not actually at nx/ny) and rx/ry may be out of bounds
		if (InBounds(rx, ry))
			pmap[ry][rx] = PMAP(ri, parts[ri].type);
	}
	return 1;
}

// try to move particle, and if successful update pmap and parts[i].x,y
int Simulation::do_move(int i, int x, int y, float nxf, float nyf)
{
	int nx = (int)(nxf+0.5f), ny = (int)(nyf+0.5f), result;
	if (edgeMode == EDGE_LOOP)
	{
		bool x_ok = (nx >= CELL && nx < XRES-CELL);
		bool y_ok = (ny >= CELL && ny < YRES-CELL);
		if (!x_ok)
			nxf = remainder_p(nxf-CELL+.5f, XRES-CELL*2.0f)+CELL-.5f;
		if (!y_ok)
			nyf = remainder_p(nyf-CELL+.5f, YRES-CELL*2.0f)+CELL-.5f;
		nx = (int)(nxf+0.5f);
		ny = (int)(nyf+0.5f);

		/*if (!x_ok || !y_ok)
		{
			//make sure there isn't something blocking it on the other side
			//only needed if this if statement is moved after the try_move (like my mod)
			//if (!eval_move(t, nx, ny, NULL) || (t == PT_PHOT && pmap[ny][nx]))
			//	return -1;
		}*/
	}
	if (parts[i].type == PT_NONE)
		return 0;
	result = try_move(i, x, y, nx, ny);
	if (result)
	{
		if (!move(i, x, y, nxf, nyf))
			return -1;
	}
	return result;
}

bool Simulation::move(int i, int x, int y, float nxf, float nyf)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int nx = (int)(nxf+0.5f), ny = (int)(nyf+0.5f);
	int t = parts[i].type;
	parts[i].x = nxf;
	parts[i].y = nyf;
	if (ny != y || nx != x)
	{
		if (pmap[y][x] && ID(pmap[y][x]) == i)
			pmap[y][x] = 0;
		if (photons[y][x] && ID(photons[y][x]) == i)
			photons[y][x] = 0;
		// kill_part if particle is out of bounds
		if (nx < CELL || nx >= XRES - CELL || ny < CELL || ny >= YRES - CELL)
		{
			kill_part(i);
			return false;
		}
		if (elements[t].Properties & TYPE_ENERGY)
			photons[ny][nx] = PMAP(i, t);
		else if (t)
			pmap[ny][nx] = PMAP(i, t);
	}

	return true;
}

void Simulation::photoelectric_effect(int nx, int ny)//create sparks from PHOT when hitting PSCN and NSCN
{
	unsigned r = pmap[ny][nx];

	if (TYP(r) == PT_PSCN && !parts[ID(r)].life)
	{
		if (TYP(pmap[ny][nx-1]) == PT_NSCN || TYP(pmap[ny][nx+1]) == PT_NSCN ||
		        TYP(pmap[ny-1][nx]) == PT_NSCN ||  TYP(pmap[ny+1][nx]) == PT_NSCN)
		{
			parts[ID(r)].ctype = PT_PSCN;
			part_change_type(ID(r), nx, ny, PT_SPRK);
			parts[ID(r)].life = 4;
		}
	}
}

unsigned static direction_to_map(float dx, float dy, int t)
{
	// TODO:
	// Adding extra directions causes some inaccuracies.
	// Not adding them causes problems with some diagonal surfaces (photons absorbed instead of reflected).
	// For now, don't add them.
	// Solution may involve more intelligent setting of initial i0 value in find_next_boundary?
	// or rewriting normal/boundary finding code

	return (dx >= 0) |
		   (((dx + dy) >= 0) << 1) |     /*  567  */
		   ((dy >= 0) << 2) |            /*  4+0  */
		   (((dy - dx) >= 0) << 3) |     /*  321  */
		   ((dx <= 0) << 4) |
		   (((dx + dy) <= 0) << 5) |
		   ((dy <= 0) << 6) |
		   (((dy - dx) <= 0) << 7);
	/*
	return (dx >= -0.001) |
		   (((dx + dy) >= -0.001) << 1) |     //  567
		   ((dy >= -0.001) << 2) |            //  4+0
		   (((dy - dx) >= -0.001) << 3) |     //  321
		   ((dx <= 0.001) << 4) |
		   (((dx + dy) <= 0.001) << 5) |
		   ((dy <= 0.001) << 6) |
		   (((dy - dx) <= 0.001) << 7);
	}*/
}

int Simulation::is_blocking(int t, int x, int y) const
{
	if (t & REFRACT) {
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return 0;
		if (TYP(pmap[y][x]) == PT_GLAS || TYP(pmap[y][x]) == PT_BGLA)
			return 1;
		return 0;
	}

	return !eval_move(t, x, y, NULL);
}

int Simulation::is_boundary(int pt, int x, int y) const
{
	if (!is_blocking(pt,x,y))
		return 0;
	if (is_blocking(pt,x,y-1) && is_blocking(pt,x,y+1) && is_blocking(pt,x-1,y) && is_blocking(pt,x+1,y))
		return 0;
	return 1;
}

int Simulation::find_next_boundary(int pt, int *x, int *y, int dm, int *em, bool reverse) const
{
	static int dx[8] = {1,1,0,-1,-1,-1,0,1};
	static int dy[8] = {0,1,1,1,0,-1,-1,-1};
	static int de[8] = {0x83,0x07,0x0E,0x1C,0x38,0x70,0xE0,0xC1};

	if (*x <= 0 || *x >= XRES-1 || *y <= 0 || *y >= YRES-1)
	{
		return 0;
	}

	if (*em != -1)
	{
		dm &= de[*em];
	}

	unsigned int mask = 0;
	for (int i = 0; i < 8; ++i)
	{
		if ((dm & (1U << i)) && is_blocking(pt, *x + dx[i], *y + dy[i]))
		{
			mask |= (1U << i);
		}
	}
	for (int i = 0; i < 8; ++i)
	{
		int n = (i + (reverse ? 1 : -1)) & 7;
		if (((mask & (1U << i))) && !(mask & (1U << n)))
		{
			*x += dx[i];
			*y += dy[i];
			*em = i;
			return 1;
		}
	}

	return 0;
}

Simulation::GetNormalResult Simulation::get_normal(int pt, int x, int y, float dx, float dy) const
{
	int ldm, rdm, lm, rm;
	int lx, ly, lv, rx, ry, rv;
	int i, j;
	float r, ex, ey;

	if (!dx && !dy)
		return { false };

	if (!is_boundary(pt, x, y))
		return { false };

	ldm = (pt & REFRACT) ? 0xFF : direction_to_map(-dy, dx, pt);
	rdm = (pt & REFRACT) ? 0xFF : direction_to_map(dy, -dx, pt);
	lx = rx = x;
	ly = ry = y;
	lv = rv = 1;
	lm = rm = -1;

	j = 0;
	for (i=0; i<SURF_RANGE; i++) {
		if (lv)
			lv = find_next_boundary(pt, &lx, &ly, ldm, &lm, true);
		if (rv)
			rv = find_next_boundary(pt, &rx, &ry, rdm, &rm, false);
		j += lv + rv;
		if (!lv && !rv)
			break;
	}

	if (j < NORMAL_MIN_EST)
		return { false };

	if ((lx == rx) && (ly == ry))
		return { false };
	ex = float(rx - lx);
	ey = float(ry - ly);
	r = 1.0f/hypot(ex, ey);
	auto nx =  ey * r;
	auto ny = -ex * r;

	return { true, nx, ny, lx, ly, rx, ry };
}



template<bool PhotoelectricEffect, class Sim>
void PhotoelectricEffectHelper(Sim &sim, int x, int y);

template<>
void PhotoelectricEffectHelper<false, const Simulation>(const Simulation &sim, int x, int y)
{
}

template<>
void PhotoelectricEffectHelper<true, Simulation>(Simulation &sim, int x, int y)
{
	sim.photoelectric_effect(x, y);
}

template<bool PhotoelectricEffect, class Sim>
Simulation::GetNormalResult Simulation::get_normal_interp(Sim &sim, int pt, float x0, float y0, float dx, float dy)
{
	int x, y, i;

	dx /= NORMAL_FRAC;
	dy /= NORMAL_FRAC;

	for (i=0; i<NORMAL_INTERP; i++) {
		x = (int)(x0 + 0.5f);
		y = (int)(y0 + 0.5f);
		if (x < 0 || y < 0 || x >= XRES || y >= YRES)
		{
			return { false };
		}
		if (sim.is_boundary(pt, x, y))
			break;
		x0 += dx;
		y0 += dy;
	}
	if (i >= NORMAL_INTERP)
		return { false };

	if (pt == PT_PHOT)
		PhotoelectricEffectHelper<PhotoelectricEffect, Sim>(sim, x, y);

	return sim.get_normal(pt, x, y, dx, dy);
}

template
Simulation::GetNormalResult Simulation::get_normal_interp<false, const Simulation>(const Simulation &sim, int pt, float x0, float y0, float dx, float dy);

void Simulation::kill_part(int i)//kills particle number i
{
	if (i < 0 || i >= NPART)
		return;
	
	int x = (int)(parts[i].x + 0.5f);
	int y = (int)(parts[i].y + 0.5f);

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int t = parts[i].type;
	if (t && elements[t].ChangeType)
	{
		(*(elements[t].ChangeType))(this, i, x, y, t, PT_NONE);
	}

	if (x >= 0 && y >= 0 && x < XRES && y < YRES)
	{
		if (pmap[y][x] && ID(pmap[y][x]) == i)
			pmap[y][x] = 0;
		else if (photons[y][x] && ID(photons[y][x]) == i)
			photons[y][x] = 0;
	}

	// This shouldn't happen but ... you never know?
	if (t == PT_NONE)
		return;

	elementCount[t]--;

	parts[i].type = PT_NONE;
	parts[i].life = pfree;
	pfree = i;
}

// Changes the type of particle number i, to t.  This also changes pmap at the same time
// Returns true if the particle was killed
bool Simulation::part_change_type(int i, int x, int y, int t)
{
	if (x<0 || y<0 || x>=XRES || y>=YRES || i>=NPART || t<0 || t>=PT_NUM || !parts[i].type)
		return false;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (!elements[t].Enabled || t == PT_NONE)
	{
		kill_part(i);
		return true;
	}
	if (elements[t].CreateAllowed)
	{
		if (!(*(elements[t].CreateAllowed))(this, i, x, y, t))
			return false;
	}

	if (elements[parts[i].type].ChangeType)
		(*(elements[parts[i].type].ChangeType))(this, i, x, y, parts[i].type, t);
	if (elements[t].ChangeType)
		(*(elements[t].ChangeType))(this, i, x, y, parts[i].type, t);

	if (parts[i].type > 0 && parts[i].type < PT_NUM && elementCount[parts[i].type])
		elementCount[parts[i].type]--;
	elementCount[t]++;

	parts[i].type = t;
	if (elements[t].Properties & TYPE_ENERGY)
	{
		photons[y][x] = PMAP(i, t);
		if (pmap[y][x] && ID(pmap[y][x]) == i)
			pmap[y][x] = 0;
	}
	else
	{
		pmap[y][x] = PMAP(i, t);
		if (photons[y][x] && ID(photons[y][x]) == i)
			photons[y][x] = 0;
	}
	return false;
}

//the function for creating a particle, use p=-1 for creating a new particle, -2 is from a brush, or a particle number to replace a particle.
//tv = Type (PMAPBITS bits) + Var (32-PMAPBITS bits), var is usually 0
int Simulation::create_part(int p, int x, int y, int t, int v)
{
	int i, oldType = PT_NONE;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (x<0 || y<0 || x>=XRES || y>=YRES || t<=0 || t>=PT_NUM || !elements[t].Enabled)
		return -1;

	if (t == PT_SPRK && !(p == -2 && elements[TYP(pmap[y][x])].CtypeDraw))
	{
		int type = TYP(pmap[y][x]);
		int index = ID(pmap[y][x]);
		if(type == PT_WIRE)
		{
			parts[index].ctype = PT_DUST;
			return index;
		}
		if (!(type == PT_INST || (elements[type].Properties&PROP_CONDUCTS)) || parts[index].life!=0)
			return -1;
		if (p == -2 && type == PT_INST)
		{
			FloodINST(x, y);
			return index;
		}
		parts[index].type = PT_SPRK;
		parts[index].life = 4;
		parts[index].ctype = type;
		pmap[y][x] = (pmap[y][x]&~PMAPMASK) | PT_SPRK;
		if (parts[index].temp+10.0f < 673.0f && !legacy_enable && (type==PT_METL || type == PT_BMTL || type == PT_BRMT || type == PT_PSCN || type == PT_NSCN || type == PT_ETRD || type == PT_NBLE || type == PT_IRON))
			parts[index].temp = parts[index].temp+10.0f;
		return index;
	}

	if (p == -2)
	{
		if (pmap[y][x])
		{
			int drawOn = TYP(pmap[y][x]);
			if (elements[drawOn].CtypeDraw)
				elements[drawOn].CtypeDraw(this, ID(pmap[y][x]), t, v);
			return -1;
		}
		else if (IsWallBlocking(x, y, t))
			return -1;
		else if (photons[y][x] && (elements[t].Properties & TYPE_ENERGY))
			return -1;
	}

	if (elements[t].CreateAllowed)
	{
		if (!(*(elements[t].CreateAllowed))(this, p, x, y, t))
			return -1;
	}

	if (p == -1)//creating from anything but brush
	{
		// If there is a particle, only allow creation if the new particle can occupy the same space as the existing particle
		// If there isn't a particle but there is a wall, check whether the new particle is allowed to be in it
		//   (not "!=2" for wall check because eval_move returns 1 for moving into empty space)
		// If there's no particle and no wall, assume creation is allowed
		if (pmap[y][x] ? (eval_move(t, x, y, NULL) != 2) : (bmap[y/CELL][x/CELL] && eval_move(t, x, y, NULL) == 0))
		{
			return -1;
		}
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else if (p == -2)//creating from brush
	{
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else if (p == -3)//skip pmap checks, e.g. for sing explosion
	{
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else
	{
		int oldX = (int)(parts[p].x + 0.5f);
		int oldY = (int)(parts[p].y + 0.5f);
		if (pmap[oldY][oldX] && ID(pmap[oldY][oldX]) == p)
			pmap[oldY][oldX] = 0;
		if (photons[oldY][oldX] && ID(photons[oldY][oldX]) == p)
			photons[oldY][oldX] = 0;

		oldType = parts[p].type;

		if (elements[oldType].ChangeType)
			(*(elements[oldType].ChangeType))(this, p, oldX, oldY, oldType, t);
		if (oldType)
			elementCount[oldType]--;

		i = p;
	}

	if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;

	parts[i] = elements[t].DefaultProperties;
	parts[i].type = t;
	parts[i].x = (float)x;
	parts[i].y = (float)y;

	//and finally set the pmap/photon maps to the newly created particle
	if (elements[t].Properties & TYPE_ENERGY)
		photons[y][x] = PMAP(i, t);
	else if (t!=PT_STKM && t!=PT_STKM2 && t!=PT_FIGH)
		pmap[y][x] = PMAP(i, t);

	//Fancy dust effects for powder types
	if((elements[t].Properties & TYPE_PART) && pretty_powder)
	{
		int colr, colg, colb;
		int sandcolourToUse = p == -2 ? sandcolour_interface : sandcolour;
		RGB<uint8_t> colour = elements[t].Colour;
		colr = colour.Red   + int(sandcolourToUse * 1.3) + rng.between(-20, 20) + rng.between(-15, 15);
		colg = colour.Green + int(sandcolourToUse * 1.3) + rng.between(-20, 20) + rng.between(-15, 15);
		colb = colour.Blue  + int(sandcolourToUse * 1.3) + rng.between(-20, 20) + rng.between(-15, 15);
		colr = std::clamp(colr, 0, 255);
		colg = std::clamp(colg, 0, 255);
		colb = std::clamp(colb, 0, 255);
		parts[i].dcolour = (rng.between(0, 149)<<24) | (colr<<16) | (colg<<8) | colb;
	}

	// Set non-static properties (such as randomly generated ones)
	if (elements[t].Create)
		(*(elements[t].Create))(this, i, x, y, t, v);

	if (elements[t].ChangeType)
		(*(elements[t].ChangeType))(this, i, x, y, oldType, t);

	elementCount[t]++;
	return i;
}

void Simulation::GetGravityField(int x, int y, float particleGrav, float newtonGrav, float & pGravX, float & pGravY)
{
	switch (gravityMode)
	{
	default:
	case GRAV_VERTICAL: //normal, vertical gravity
		pGravX = 0;
		pGravY = particleGrav;
		break;
	case GRAV_OFF: //no gravity
		pGravX = 0;
		pGravY = 0;
		break;
	case GRAV_RADIAL: //radial gravity
		{
			pGravX = 0;
			pGravY = 0;
			auto dx = float(x - XCNTR);
			auto dy = float(y - YCNTR);
			if (dx || dy)
			{
				auto pGravD = 0.01f - hypotf(dx, dy);
				pGravX = particleGrav * (dx / pGravD);
				pGravY = particleGrav * (dy / pGravD);
			}
		}
		break;
	case GRAV_CUSTOM: //custom gravity
		pGravX = particleGrav * customGravityX;
		pGravY = particleGrav * customGravityY;
		break;
	}
	if (newtonGrav)
	{
		pGravX += newtonGrav*gravx[(y/CELL)*XCELLS+(x/CELL)];
		pGravY += newtonGrav*gravy[(y/CELL)*XCELLS+(x/CELL)];
	}
}

void Simulation::create_gain_photon(int pp)//photons from PHOT going through GLOW
{
	float xx, yy;
	int i, lr, temp_bin, nx, ny;

	if (pfree == -1)
		return;
	i = pfree;

	lr = 2*rng.between(0, 1) - 1; // -1 or 1

	xx = parts[pp].x - lr*0.3*parts[pp].vy;
	yy = parts[pp].y + lr*0.3*parts[pp].vx;

	nx = (int)(xx + 0.5f);
	ny = (int)(yy + 0.5f);

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return;

	if (TYP(pmap[ny][nx]) != PT_GLOW)
		return;

	pfree = parts[i].life;
	if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;

	parts[i].type = PT_PHOT;
	parts[i].life = 680;
	parts[i].x = xx;
	parts[i].y = yy;
	parts[i].vx = parts[pp].vx;
	parts[i].vy = parts[pp].vy;
	parts[i].temp = parts[ID(pmap[ny][nx])].temp;
	parts[i].tmp = 0;
	parts[i].tmp3 = 0;
	parts[i].tmp4 = 0;
	photons[ny][nx] = PMAP(i, PT_PHOT);

	temp_bin = (int)((parts[i].temp-273.0f)*0.25f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	parts[i].ctype = 0x1F << temp_bin;
}

void Simulation::create_cherenkov_photon(int pp)//photons from NEUT going through GLAS
{
	int i, lr, nx, ny;
	float r;

	if (pfree == -1)
		return;
	i = pfree;

	nx = (int)(parts[pp].x + 0.5f);
	ny = (int)(parts[pp].y + 0.5f);
	if (TYP(pmap[ny][nx]) != PT_GLAS && TYP(pmap[ny][nx]) != PT_BGLA)
		return;

	if (hypotf(parts[pp].vx, parts[pp].vy) < 1.44f)
		return;

	pfree = parts[i].life;
	if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;

	lr = rng.between(0, 1);

	parts[i].type = PT_PHOT;
	parts[i].ctype = 0x00000F80;
	parts[i].life = 680;
	parts[i].x = parts[pp].x;
	parts[i].y = parts[pp].y;
	parts[i].temp = parts[ID(pmap[ny][nx])].temp;
	parts[i].tmp = 0;
	parts[i].tmp3 = 0;
	parts[i].tmp4 = 0;
	photons[ny][nx] = PMAP(i, PT_PHOT);

	if (lr) {
		parts[i].vx = parts[pp].vx - 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy + 2.5f*parts[pp].vx;
	} else {
		parts[i].vx = parts[pp].vx + 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy - 2.5f*parts[pp].vx;
	}

	/* photons have speed of light. no discussion. */
	r = 1.269 / hypotf(parts[i].vx, parts[i].vy);
	parts[i].vx *= r;
	parts[i].vy *= r;
}

void Simulation::delete_part(int x, int y)//calls kill_part with the particle located at x,y
{
	unsigned i;

	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return;

	i = photons[y][x] ? photons[y][x] : pmap[y][x];

	if (!i)
		return;
	kill_part(ID(i));
}

template<bool UpdateEmap, class Sim>
void UpdateEmapHelper(Sim &sim, int fin_x, int fin_y);

template<>
void UpdateEmapHelper<false, const Simulation>(const Simulation &sim, int fin_x, int fin_y)
{
}

template<>
void UpdateEmapHelper<true, Simulation>(Simulation &sim, int fin_x, int fin_y)
{
	if (sim.bmap[fin_y/CELL][fin_x/CELL]==WL_DETECT && sim.emap[fin_y/CELL][fin_x/CELL]<8)
		sim.set_emap(fin_x/CELL, fin_y/CELL);
}

template<bool UpdateEmap, class Sim>
Simulation::PlanMoveResult Simulation::PlanMove(Sim &sim, int i, int x, int y)
{
	auto &parts = sim.parts;
	auto &bmap = sim.bmap;
	auto &emap = sim.emap;
	auto &pmap = sim.pmap;
	auto edgeMode = sim.edgeMode;
	auto &sd = SimulationData::CRef();
	auto &can_move = sd.can_move;
	auto t = parts[i].type;
	int fin_x, fin_y, clear_x, clear_y;
	float fin_xf, fin_yf, clear_xf, clear_yf;
	auto vx = parts[i].vx;
	auto vy = parts[i].vy;
	auto mv = fmaxf(fabsf(vx), fabsf(vy));
	if (mv < ISTP)
	{
		clear_x = x;
		clear_y = y;
		clear_xf = parts[i].x;
		clear_yf = parts[i].y;
		fin_xf = clear_xf + vx;
		fin_yf = clear_yf + vy;
		fin_x = (int)(fin_xf+0.5f);
		fin_y = (int)(fin_yf+0.5f);
	}
	else
	{
		if (mv > MAX_VELOCITY)
		{
			vx *= MAX_VELOCITY/mv;
			vy *= MAX_VELOCITY/mv;
			mv = MAX_VELOCITY;
		}
		// interpolate to see if there is anything in the way
		auto dx = vx*ISTP/mv;
		auto dy = vy*ISTP/mv;
		fin_xf = parts[i].x;
		fin_yf = parts[i].y;
		fin_x = (int)(fin_xf+0.5f);
		fin_y = (int)(fin_yf+0.5f);
		bool closedEholeStart = InBounds(fin_x, fin_y) && (bmap[fin_y/CELL][fin_x/CELL] == WL_EHOLE && !emap[fin_y/CELL][fin_x/CELL]);
		while (1)
		{
			mv -= ISTP;
			fin_xf += dx;
			fin_yf += dy;
			fin_x = (int)(fin_xf+0.5f);
			fin_y = (int)(fin_yf+0.5f);
			if (edgeMode == EDGE_LOOP)
			{
				bool x_ok = (fin_xf >= CELL-.5f && fin_xf < XRES-CELL-.5f);
				bool y_ok = (fin_yf >= CELL-.5f && fin_yf < YRES-CELL-.5f);
				if (!x_ok)
					fin_xf = remainder_p(fin_xf-CELL+.5f, XRES-CELL*2.0f)+CELL-.5f;
				if (!y_ok)
					fin_yf = remainder_p(fin_yf-CELL+.5f, YRES-CELL*2.0f)+CELL-.5f;
				fin_x = (int)(fin_xf+0.5f);
				fin_y = (int)(fin_yf+0.5f);
			}
			if (mv <= 0.0f)
			{
				// nothing found
				fin_xf = parts[i].x + vx;
				fin_yf = parts[i].y + vy;
				if (edgeMode == EDGE_LOOP)
				{
					bool x_ok = (fin_xf >= CELL-.5f && fin_xf < XRES-CELL-.5f);
					bool y_ok = (fin_yf >= CELL-.5f && fin_yf < YRES-CELL-.5f);
					if (!x_ok)
						fin_xf = remainder_p(fin_xf-CELL+.5f, XRES-CELL*2.0f)+CELL-.5f;
					if (!y_ok)
						fin_yf = remainder_p(fin_yf-CELL+.5f, YRES-CELL*2.0f)+CELL-.5f;
				}
				fin_x = (int)(fin_xf+0.5f);
				fin_y = (int)(fin_yf+0.5f);
				clear_xf = fin_xf-dx;
				clear_yf = fin_yf-dy;
				clear_x = (int)(clear_xf+0.5f);
				clear_y = (int)(clear_yf+0.5f);
				break;
			}
			//block if particle can't move (0), or some special cases where it returns 1 (can_move = 3 but returns 1 meaning particle will be eaten)
			//also photons are still blocked (slowed down) by any particle (even ones it can move through), and absorb wall also blocks particles
			int eval = sim.eval_move(t, fin_x, fin_y, NULL);
			if (!eval || (can_move[t][TYP(pmap[fin_y][fin_x])] == 3 && eval == 1) || (t == PT_PHOT && pmap[fin_y][fin_x]) || bmap[fin_y/CELL][fin_x/CELL]==WL_DESTROYALL || closedEholeStart!=(bmap[fin_y/CELL][fin_x/CELL] == WL_EHOLE && !emap[fin_y/CELL][fin_x/CELL]))
			{
				// found an obstacle
				clear_xf = fin_xf-dx;
				clear_yf = fin_yf-dy;
				clear_x = (int)(clear_xf+0.5f);
				clear_y = (int)(clear_yf+0.5f);
				break;
			}
			UpdateEmapHelper<UpdateEmap, Sim>(sim, fin_x, fin_y);
		}
	}
	return {
		fin_x,
		fin_y,
		clear_x,
		clear_y,
		fin_xf,
		fin_yf,
		clear_xf,
		clear_yf,
		vx,
		vy,
	};
}

template
Simulation::PlanMoveResult Simulation::PlanMove<false, const Simulation>(const Simulation &sim, int i, int x, int y);

void Simulation::UpdateParticles(int start, int end)
{
	//the main particle loop function, goes over all particles.
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	for (auto i = start; i < end && i <= parts_lastActiveIndex; i++)
	{
		if (parts[i].type)
		{
			debug_mostRecentlyUpdated = i;
			auto t = parts[i].type;

			auto x = (int)(parts[i].x+0.5f);
			auto y = (int)(parts[i].y+0.5f);

			// Kill a particle off screen
			if (x<CELL || y<CELL || x>=XRES-CELL || y>=YRES-CELL)
			{
				kill_part(i);
				continue;
			}

			// Kill a particle in a wall where it isn't supposed to go
			if (bmap[y/CELL][x/CELL] &&
			   (bmap[y/CELL][x/CELL]==WL_WALL ||
			    bmap[y/CELL][x/CELL]==WL_WALLELEC ||
			    bmap[y/CELL][x/CELL]==WL_ALLOWAIR ||
			    (bmap[y/CELL][x/CELL]==WL_DESTROYALL) ||
			    (bmap[y/CELL][x/CELL]==WL_ALLOWLIQUID && !(elements[t].Properties&TYPE_LIQUID)) ||
			    (bmap[y/CELL][x/CELL]==WL_ALLOWPOWDER && !(elements[t].Properties&TYPE_PART)) ||
			    (bmap[y/CELL][x/CELL]==WL_ALLOWGAS && !(elements[t].Properties&TYPE_GAS)) || //&& elements[t].Falldown!=0 && parts[i].type!=PT_FIRE && parts[i].type!=PT_SMKE && parts[i].type!=PT_CFLM) ||
			            (bmap[y/CELL][x/CELL]==WL_ALLOWENERGY && !(elements[t].Properties&TYPE_ENERGY)) ||
			    (bmap[y/CELL][x/CELL]==WL_EWALL && !emap[y/CELL][x/CELL])) && (t!=PT_STKM) && (t!=PT_STKM2) && (t!=PT_FIGH))
			{
				kill_part(i);
				continue;
			}

			// Make sure that STASIS'd particles don't tick.
			if (bmap[y/CELL][x/CELL] == WL_STASIS && emap[y/CELL][x/CELL]<8) {
				continue;
			}

			if (bmap[y/CELL][x/CELL]==WL_DETECT && emap[y/CELL][x/CELL]<8)
				set_emap(x/CELL, y/CELL);

			//adding to velocity from the particle's velocity
			vx[y/CELL][x/CELL] = vx[y/CELL][x/CELL]*elements[t].AirLoss + elements[t].AirDrag*parts[i].vx;
			vy[y/CELL][x/CELL] = vy[y/CELL][x/CELL]*elements[t].AirLoss + elements[t].AirDrag*parts[i].vy;

			if (elements[t].HotAir)
			{
				if (t==PT_GAS||t==PT_NBLE)
				{
					if (pv[y/CELL][x/CELL]<3.5f)
						pv[y/CELL][x/CELL] += elements[t].HotAir*(3.5f-pv[y/CELL][x/CELL]);
					if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<3.5f)
						pv[y/CELL+1][x/CELL] += elements[t].HotAir*(3.5f-pv[y/CELL+1][x/CELL]);
					if (x+CELL<XRES)
					{
						if (pv[y/CELL][x/CELL+1]<3.5f)
							pv[y/CELL][x/CELL+1] += elements[t].HotAir*(3.5f-pv[y/CELL][x/CELL+1]);
						if (y+CELL<YRES && pv[y/CELL+1][x/CELL+1]<3.5f)
							pv[y/CELL+1][x/CELL+1] += elements[t].HotAir*(3.5f-pv[y/CELL+1][x/CELL+1]);
					}
				}
				else//add the hotair variable to the pressure map, like black hole, or white hole.
				{
					pv[y/CELL][x/CELL] += elements[t].HotAir;
					if (y+CELL<YRES)
						pv[y/CELL+1][x/CELL] += elements[t].HotAir;
					if (x+CELL<XRES)
					{
						pv[y/CELL][x/CELL+1] += elements[t].HotAir;
						if (y+CELL<YRES)
							pv[y/CELL+1][x/CELL+1] += elements[t].HotAir;
					}
				}
			}

			float pGravX = 0, pGravY = 0;
			if (!(elements[t].Properties & TYPE_SOLID) && (elements[t].Gravity || elements[t].NewtonianGravity))
			{
				GetGravityField(x, y, elements[t].Gravity, elements[t].NewtonianGravity, pGravX, pGravY);
			}

			//velocity updates for the particle
			if (t != PT_SPNG || !(parts[i].flags&FLAG_MOVABLE))
			{
				parts[i].vx *= elements[t].Loss;
				parts[i].vy *= elements[t].Loss;
			}
			//particle gets velocity from the vx and vy maps
			parts[i].vx += elements[t].Advection*vx[y/CELL][x/CELL] + pGravX;
			parts[i].vy += elements[t].Advection*vy[y/CELL][x/CELL] + pGravY;


			if (elements[t].Diffusion)//the random diffusion that gasses have
			{
				if constexpr (LATENTHEAT)
				{
					//The magic number controls diffusion speed
					parts[i].vx += 0.05*sqrtf(parts[i].temp)*elements[t].Diffusion*(2.0f*rng.uniform01()-1.0f);
					parts[i].vy += 0.05*sqrtf(parts[i].temp)*elements[t].Diffusion*(2.0f*rng.uniform01()-1.0f);
				}
				else
				{
					parts[i].vx += elements[t].Diffusion*(2.0f*rng.uniform01()-1.0f);
					parts[i].vy += elements[t].Diffusion*(2.0f*rng.uniform01()-1.0f);
				}
			}

			auto transitionOccurred = false;

			int surround[8];
			auto surround_space = 0;
			auto nt = 0; //if nt is greater than 1 after this, then there is a particle around the current particle, that is NOT the current particle's type, for water movement.
			{
				auto j = 0;
				for (auto nx=-1; nx<2; nx++)
				{
					for (auto ny=-1; ny<2; ny++)
					{
						if (nx||ny)
						{
							auto r = pmap[y+ny][x+nx];
							surround[j] = r;
							j++;
							surround_space += (!TYP(r)); // count empty space
							nt += (TYP(r)!=t); // count empty space and particles of different type
						}
					}
				}
			}

			float gel_scale = 1.0f;
			if (t==PT_GEL)
				gel_scale = parts[i].tmp*2.55f;

			if (!legacy_enable)
			{
				if ((elements[t].Properties&TYPE_LIQUID) && (t!=PT_GEL || gel_scale > (1 + rng.between(0, 254))))
				{
					float convGravX, convGravY;
					GetGravityField(x, y, -2.0f, -2.0f, convGravX, convGravY);
					auto offsetX = int(std::round(convGravX + x));
					auto offsetY = int(std::round(convGravY + y));
					if ((offsetX != x || offsetY != y) && offsetX >= 0 && offsetX < XRES && offsetY >= 0 && offsetY < YRES) {//some heat convection for liquids
						auto r = pmap[offsetY][offsetX];
						if (!(!r || parts[i].type != TYP(r))) {
							if (parts[i].temp>parts[ID(r)].temp) {
								auto swappage = parts[i].temp;
								parts[i].temp = parts[ID(r)].temp;
								parts[ID(r)].temp = swappage;
							}
						}
					}
				}

				//heat transfer code
				auto h_count = 0;
				bool cond;
				if constexpr (LATENTHEAT)
				{
					cond = t && (t!=PT_HSWC||parts[i].life==10) && elements[t].HeatConduct*gel_scale > 0;
				}
				else
				{
					cond = t && (t!=PT_HSWC||parts[i].life==10) && rng.chance(int(elements[t].HeatConduct*gel_scale), 250);
				}
				if (cond)
				{
					if (aheat_enable && !(elements[t].Properties&PROP_NOAMBHEAT))
					{
						if constexpr (LATENTHEAT)
						{
							auto c_heat = parts[i].temp*96.645/elements[t].HeatConduct*gel_scale*std::fabs(elements[t].Weight) + hv[y/CELL][x/CELL]*100*(pv[y/CELL][x/CELL]-MIN_PRESSURE)/(MAX_PRESSURE-MIN_PRESSURE)*2;
							float c_Cm = 96.645/elements[t].HeatConduct*gel_scale*std::fabs(elements[t].Weight) + 100*(pv[y/CELL][x/CELL]-MIN_PRESSURE)/(MAX_PRESSURE-MIN_PRESSURE)*2;
							auto pt = c_heat/c_Cm;
							pt = restrict_flt(pt, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
							parts[i].temp = pt;
							//Pressure increase from heat (temporary)
							pv[y/CELL][x/CELL] += (pt-hv[y/CELL][x/CELL])*0.004;
							hv[y/CELL][x/CELL] = pt;
						}
						else
						{
							auto c_heat = (hv[y/CELL][x/CELL]-parts[i].temp)*0.04;
							c_heat = restrict_flt(c_heat, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
							parts[i].temp += c_heat;
							hv[y/CELL][x/CELL] -= c_heat;
						}
					}
					auto c_heat = 0.0f;
					float c_Cm = 0.0f;
					int surround_hconduct[8];
					for (auto j=0; j<8; j++)
					{
						surround_hconduct[j] = i;
						auto r = surround[j];
						if (!r)
							continue;
						auto rt = TYP(r);
						if (rt && elements[rt].HeatConduct && (rt!=PT_HSWC||parts[ID(r)].life==10)
						        && (t!=PT_FILT||(rt!=PT_BRAY&&rt!=PT_BIZR&&rt!=PT_BIZRG))
						        && (rt!=PT_FILT||(t!=PT_BRAY&&t!=PT_PHOT&&t!=PT_BIZR&&t!=PT_BIZRG))
						        && (t!=PT_ELEC||rt!=PT_DEUT)
						        && (t!=PT_DEUT||rt!=PT_ELEC)
						        && (t!=PT_HSWC || rt!=PT_FILT || parts[i].tmp != 1)
						        && (t!=PT_FILT || rt!=PT_HSWC || parts[ID(r)].tmp != 1))
						{
							surround_hconduct[j] = ID(r);
							if constexpr (LATENTHEAT)
							{
								if (rt==PT_GEL)
									gel_scale = parts[ID(r)].tmp*2.55f;
								else gel_scale = 1.0f;

								c_heat += parts[ID(r)].temp*96.645/elements[rt].HeatConduct*gel_scale*std::fabs(elements[rt].Weight);
								c_Cm += 96.645/elements[rt].HeatConduct*gel_scale*std::fabs(elements[rt].Weight);
							}
							else
							{
								c_heat += parts[ID(r)].temp;
							}
							h_count++;
						}
					}
					float pt = R_TEMP;
					if constexpr (LATENTHEAT)
					{
						if (t==PT_GEL)
							gel_scale = parts[i].tmp*2.55f;
						else gel_scale = 1.0f;

						if (t == PT_PHOT)
							pt = (c_heat+parts[i].temp*96.645)/(c_Cm+96.645);
						else
							pt = (c_heat+parts[i].temp*96.645/elements[t].HeatConduct*gel_scale*std::fabs(elements[t].Weight))/(c_Cm+96.645/elements[t].HeatConduct*gel_scale*std::fabs(elements[t].Weight));

						c_heat += parts[i].temp*96.645/elements[t].HeatConduct*gel_scale*std::fabs(elements[t].Weight);
						c_Cm += 96.645/elements[t].HeatConduct*gel_scale*std::fabs(elements[t].Weight);
						parts[i].temp = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
					}
					else
					{
						pt = (c_heat+parts[i].temp)/(h_count+1);
						pt = parts[i].temp = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
						for (auto j=0; j<8; j++)
						{
							parts[surround_hconduct[j]].temp = pt;
						}
					}

					auto ctemph = pt;
					auto ctempl = pt;
					// change boiling point with pressure
					if (((elements[t].Properties&TYPE_LIQUID) && sd.IsElementOrNone(elements[t].HighTemperatureTransition) && (elements[elements[t].HighTemperatureTransition].Properties&TYPE_GAS))
					        || t==PT_LNTG || t==PT_SLTW)
						ctemph -= 2.0f*pv[y/CELL][x/CELL];
					else if (((elements[t].Properties&TYPE_GAS) && sd.IsElementOrNone(elements[t].LowTemperatureTransition) && (elements[elements[t].LowTemperatureTransition].Properties&TYPE_LIQUID))
					         || t==PT_WTRV)
						ctempl -= 2.0f*pv[y/CELL][x/CELL];
					auto s = 1;

					//A fix for ice with ctype = 0
					if ((t==PT_ICEI || t==PT_SNOW) && (!sd.IsElement(parts[i].ctype) || parts[i].ctype==PT_ICEI || parts[i].ctype==PT_SNOW))
						parts[i].ctype = PT_WATR;

					if (elements[t].HighTemperatureTransition>-1 && ctemph>=elements[t].HighTemperature)
					{
						// particle type change due to high temperature
						float dbt = ctempl - pt;
						if (elements[t].HighTemperatureTransition != PT_NUM)
						{
							if constexpr (LATENTHEAT)
							{
								if (elements[t].LatentHeat <= (c_heat - (elements[t].HighTemperature - dbt)*c_Cm))
								{
									pt = (c_heat - elements[t].LatentHeat)/c_Cm;
									t = elements[t].HighTemperatureTransition;
								}
								else
								{
									parts[i].temp = restrict_flt(elements[t].HighTemperature - dbt, MIN_TEMP, MAX_TEMP);
									s = 0;
								}
							}
							else
							{
								t = elements[t].HighTemperatureTransition;
							}
						}
						else if (t == PT_ICEI || t == PT_SNOW)
						{
							if (parts[i].ctype > 0 && parts[i].ctype < PT_NUM && parts[i].ctype != t)
							{
								if (elements[parts[i].ctype].LowTemperatureTransition==PT_ICEI || elements[parts[i].ctype].LowTemperatureTransition==PT_SNOW)
								{
									if (pt<elements[parts[i].ctype].LowTemperature)
										s = 0;
								}
								else if (pt<273.15f)
									s = 0;

								if (s)
								{
									if constexpr (LATENTHEAT)
									{
										//One ice table value for all it's kinds
										if (elements[t].LatentHeat <= (c_heat - (elements[parts[i].ctype].LowTemperature - dbt)*c_Cm))
										{
											pt = (c_heat - elements[t].LatentHeat)/c_Cm;
											t = parts[i].ctype;
											parts[i].ctype = PT_NONE;
											parts[i].life = 0;
										}
										else
										{
											parts[i].temp = restrict_flt(elements[parts[i].ctype].LowTemperature - dbt, MIN_TEMP, MAX_TEMP);
											s = 0;
										}
									}
									else
									{
										t = parts[i].ctype;
										parts[i].ctype = PT_NONE;
										parts[i].life = 0;
									}
								}
							}
							else
								s = 0;
						}
						else if (t == PT_SLTW)
						{
							if constexpr (LATENTHEAT)
							{
								if (elements[t].LatentHeat <= (c_heat - (elements[t].HighTemperature - dbt)*c_Cm))
								{
									pt = (c_heat - elements[t].LatentHeat)/c_Cm;

									t = rng.chance(1, 4) ? PT_SALT : PT_WTRV;
								}
								else
								{
									parts[i].temp = restrict_flt(elements[t].HighTemperature - dbt, MIN_TEMP, MAX_TEMP);
									s = 0;
								}
							}
							else
							{
								t = rng.chance(1, 4) ? PT_SALT : PT_WTRV;
							}
						}
						else if (t == PT_BRMT)
						{
							if (parts[i].ctype == PT_TUNG)
							{
								if (ctemph < elements[parts[i].ctype].HighTemperature)
									s = 0;
								else
								{
									t = PT_LAVA;
									parts[i].type = PT_TUNG;
								}
							}
							else if (ctemph >= elements[t].HighTemperature)
								t = PT_LAVA;
							else
								s = 0;
						}
						else if (t == PT_CRMC)
						{
							float pres = std::max((pv[y/CELL][x/CELL]+pv[(y-2)/CELL][x/CELL]+pv[(y+2)/CELL][x/CELL]+pv[y/CELL][(x-2)/CELL]+pv[y/CELL][(x+2)/CELL])*2.0f, 0.0f);
							if (ctemph < pres+elements[PT_CRMC].HighTemperature)
								s = 0;
							else
								t = PT_LAVA;
						}
						else
							s = 0;
					}
					else if (elements[t].LowTemperatureTransition > -1 && ctempl<elements[t].LowTemperature)
					{
						// particle type change due to low temperature
						float dbt = ctempl - pt;
						if (elements[t].LowTemperatureTransition != PT_NUM)
						{
							if constexpr (LATENTHEAT)
							{
								if (elements[elements[t].LowTemperatureTransition].LatentHeat >= (c_heat - (elements[t].LowTemperature - dbt)*c_Cm))
								{
									pt = (c_heat + elements[elements[t].LowTemperatureTransition].LatentHeat)/c_Cm;
									t = elements[t].LowTemperatureTransition;
								}
								else
								{
									parts[i].temp = restrict_flt(elements[t].LowTemperature - dbt, MIN_TEMP, MAX_TEMP);
									s = 0;
								}
							}
							else
							{
								t = elements[t].LowTemperatureTransition;
							}
						}
						else if (t == PT_WTRV)
						{
							t = (pt < 273.0f) ? PT_RIME : PT_DSTW;
						}
						else if (t == PT_LAVA)
						{
							if (parts[i].ctype > 0 && parts[i].ctype < PT_NUM && parts[i].ctype != PT_LAVA && elements[parts[i].ctype].Enabled)
							{
								if (parts[i].ctype == PT_THRM && pt >= elements[PT_BMTL].HighTemperature)
									s = 0;
								else if ((parts[i].ctype == PT_VIBR || parts[i].ctype == PT_BVBR) && pt >= 273.15f)
									s = 0;
								else if (parts[i].ctype == PT_TUNG)
								{
									// TUNG does its own melting in its update function, so HighTemperatureTransition is not LAVA so it won't be handled by the code for HighTemperatureTransition==PT_LAVA below
									// However, the threshold is stored in HighTemperature to allow it to be changed from Lua
									if (pt >= elements[parts[i].ctype].HighTemperature)
										s = 0;
								}
								else if (parts[i].ctype == PT_CRMC)
								{
									float pres = std::max((pv[y/CELL][x/CELL]+pv[(y-2)/CELL][x/CELL]+pv[(y+2)/CELL][x/CELL]+pv[y/CELL][(x-2)/CELL]+pv[y/CELL][(x+2)/CELL])*2.0f, 0.0f);
									if (ctemph >= pres+elements[PT_CRMC].HighTemperature)
										s = 0;
								}
								else if (elements[parts[i].ctype].HighTemperatureTransition == PT_LAVA || parts[i].ctype == PT_HEAC)
								{
									if (pt >= elements[parts[i].ctype].HighTemperature)
										s = 0;
								}
								else if (pt>=973.0f)
									s = 0; // freezing point for lava with any other (not listed in ptransitions as turning into lava) ctype
								if (s)
								{
									t = parts[i].ctype;
									parts[i].ctype = PT_NONE;
									if (t == PT_THRM)
									{
										parts[i].tmp = 0;
										t = PT_BMTL;
									}
									if (t == PT_PLUT)
									{
										parts[i].tmp = 0;
										t = PT_LAVA;
									}
								}
							}
							else if (pt<973.0f)
								t = PT_STNE;
							else
								s = 0;
						}
						else
							s = 0;
					}
					else
						s = 0;
					if constexpr (LATENTHEAT)
					{
						pt = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
						for (auto j=0; j<8; j++)
						{
							parts[surround_hconduct[j]].temp = pt;
						}
					}
					if (s) // particle type change occurred
					{
						if (t==PT_ICEI || t==PT_LAVA || t==PT_SNOW)
							parts[i].ctype = parts[i].type;
						if (!(t==PT_ICEI && parts[i].ctype==PT_FRZW))
							parts[i].life = 0;
						if (t == PT_FIRE)
						{
							//hackish, if tmp isn't 0 the FIRE might turn into DSTW later
							//idealy transitions should use create_part(i) but some elements rely on properties staying constant
							//and I don't feel like checking each one right now
							parts[i].tmp = 0;
						}
						if ((elements[t].Properties&TYPE_GAS) && !(elements[parts[i].type].Properties&TYPE_GAS))
							pv[y/CELL][x/CELL] += 0.50f;

						if (t == PT_NONE)
						{
							kill_part(i);
							goto killed;
						}
						// part_change_type could refuse to change the type and kill the particle
						// for example, changing type to STKM but one already exists
						// we need to account for that to not cause simulation corruption issues
						if (part_change_type(i,x,y,t))
							goto killed;

						if (t==PT_FIRE || t==PT_PLSM || t==PT_CFLM)
							parts[i].life = rng.between(120, 169);
						if (t == PT_LAVA)
						{
							if (parts[i].ctype == PT_BRMT) parts[i].ctype = PT_BMTL;
							else if (parts[i].ctype == PT_SAND) parts[i].ctype = PT_GLAS;
							else if (parts[i].ctype == PT_BGLA) parts[i].ctype = PT_GLAS;
							else if (parts[i].ctype == PT_PQRT) parts[i].ctype = PT_QRTZ;
							else if (parts[i].ctype == PT_LITH && parts[i].tmp2 > 3) parts[i].ctype = PT_GLAS;
							parts[i].life = rng.between(240, 359);
						}
						transitionOccurred = true;
					}

					pt = parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
					if (t == PT_LAVA)
					{
						parts[i].life = int(restrict_flt((parts[i].temp-700)/7, 0, 400));
						if (parts[i].ctype==PT_THRM&&parts[i].tmp>0)
						{
							parts[i].tmp--;
							parts[i].temp = 3500;
						}
						if (parts[i].ctype==PT_PLUT&&parts[i].tmp>0)
						{
							parts[i].tmp--;
							parts[i].temp = MAX_TEMP;
						}
					}
				}
				else
				{
					if (!(air->bmap_blockairh[y/CELL][x/CELL]&0x8))
						air->bmap_blockairh[y/CELL][x/CELL]++;
					parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
				}
			}

			if (t==PT_LIFE)
			{
				parts[i].temp = restrict_flt(parts[i].temp-50.0f, MIN_TEMP, MAX_TEMP);
			}
			if (t==PT_WIRE)
			{
				//wire_placed = 1;
			}
			//spark updates from walls
			if ((elements[t].Properties&PROP_CONDUCTS) || t==PT_SPRK)
			{
				auto nx = x % CELL;
				if (nx == 0)
					nx = x/CELL - 1;
				else if (nx == CELL-1)
					nx = x/CELL + 1;
				else
					nx = x/CELL;
				auto ny = y % CELL;
				if (ny == 0)
					ny = y/CELL - 1;
				else if (ny == CELL-1)
					ny = y/CELL + 1;
				else
					ny = y/CELL;
				if (nx>=0 && ny>=0 && nx<XCELLS && ny<YCELLS)
				{
					if (t!=PT_SPRK)
					{
						if (emap[ny][nx]==12 && !parts[i].life && bmap[ny][nx] != WL_STASIS)
						{
							part_change_type(i,x,y,PT_SPRK);
							parts[i].life = 4;
							parts[i].ctype = t;
							t = PT_SPRK;
						}
					}
					else if (bmap[ny][nx]==WL_DETECT || bmap[ny][nx]==WL_EWALL || bmap[ny][nx]==WL_ALLOWLIQUID || bmap[ny][nx]==WL_WALLELEC || bmap[ny][nx]==WL_ALLOWALLELEC || bmap[ny][nx]==WL_EHOLE)
						set_emap(nx, ny);
				}
			}

			//the basic explosion, from the .explosive variable
			if ((elements[t].Explosive&2) && pv[y/CELL][x/CELL]>2.5f)
			{
				parts[i].life = rng.between(180, 259);
				parts[i].temp = restrict_flt(elements[PT_FIRE].DefaultProperties.temp + (elements[t].Flammable/2), MIN_TEMP, MAX_TEMP);
				t = PT_FIRE;
				part_change_type(i,x,y,t);
				pv[y/CELL][x/CELL] += 0.25f * CFDS;
			}

			{
				auto s = 1;
				auto gravtot = fabs(gravy[(y/CELL)*XCELLS+(x/CELL)])+fabs(gravx[(y/CELL)*XCELLS+(x/CELL)]);
				if (elements[t].HighPressureTransition>-1 && pv[y/CELL][x/CELL]>elements[t].HighPressure) {
					// particle type change due to high pressure
					if (elements[t].HighPressureTransition!=PT_NUM)
						t = elements[t].HighPressureTransition;
					else if (t==PT_BMTL) {
						if (pv[y/CELL][x/CELL]>2.5f)
							t = PT_BRMT;
						else if (pv[y/CELL][x/CELL]>1.0f && parts[i].tmp==1)
							t = PT_BRMT;
						else s = 0;
					}
					else s = 0;
				} else if (elements[t].LowPressureTransition>-1 && pv[y/CELL][x/CELL]<elements[t].LowPressure && gravtot<=(elements[t].LowPressure/4.0f)) {
					// particle type change due to low pressure
					if (elements[t].LowPressureTransition!=PT_NUM)
						t = elements[t].LowPressureTransition;
					else s = 0;
				} else if (elements[t].HighPressureTransition>-1 && gravtot>(elements[t].HighPressure/4.0f)) {
					// particle type change due to high gravity
					if (elements[t].HighPressureTransition!=PT_NUM)
						t = elements[t].HighPressureTransition;
					else if (t==PT_BMTL) {
						if (gravtot>0.625f)
							t = PT_BRMT;
						else if (gravtot>0.25f && parts[i].tmp==1)
							t = PT_BRMT;
						else s = 0;
					}
					else s = 0;
				} else s = 0;

				// particle type change occurred
				if (s)
				{
					if (t == PT_NONE)
					{
						kill_part(i);
						goto killed;
					}
					parts[i].life = 0;
					// part_change_type could refuse to change the type and kill the particle
					// for example, changing type to STKM but one already exists
					// we need to account for that to not cause simulation corruption issues
					if (part_change_type(i,x,y,t))
						goto killed;
					if (t == PT_FIRE)
						parts[i].life = rng.between(120, 169);
					transitionOccurred = true;
				}
			}

			//call the particle update function, if there is one
			if (elements[t].Update)
			{
				if ((*(elements[t].Update))(this, i, x, y, surround_space, nt, parts, pmap))
					continue;
				x = (int)(parts[i].x+0.5f);
				y = (int)(parts[i].y+0.5f);
			}

			if(legacy_enable)//if heat sim is off
				Element::legacyUpdate(this, i,x,y,surround_space,nt, parts, pmap);

killed:
			if (parts[i].type == PT_NONE)//if its dead, skip to next particle
				continue;

			if (transitionOccurred)
				continue;

			if (!parts[i].vx&&!parts[i].vy)//if its not moving, skip to next particle, movement code it next
				continue;

			int fin_x, fin_y, clear_x, clear_y;
			float fin_xf, fin_yf, clear_xf, clear_yf;
			{
				auto mr = PlanMove<true>(*this, i, x, y);
				fin_x    = mr.fin_x;
				fin_y    = mr.fin_y;
				clear_x  = mr.clear_x;
				clear_y  = mr.clear_y;
				fin_xf   = mr.fin_xf;
				fin_yf   = mr.fin_yf;
				clear_xf = mr.clear_xf;
				clear_yf = mr.clear_yf;
				parts[i].vx = mr.vx;
				parts[i].vy = mr.vy;
			}

			auto stagnant = parts[i].flags & FLAG_STAGNANT;
			parts[i].flags &= ~FLAG_STAGNANT;

			if (t==PT_STKM || t==PT_STKM2 || t==PT_FIGH)
			{
				//head movement, let head pass through anything
				parts[i].x += parts[i].vx;
				parts[i].y += parts[i].vy;
				int nx = (int)((float)parts[i].x+0.5f);
				int ny = (int)((float)parts[i].y+0.5f);
				if (edgeMode == EDGE_LOOP)
				{
					bool x_ok = (nx >= CELL && nx < XRES-CELL);
					bool y_ok = (ny >= CELL && ny < YRES-CELL);
					int oldnx = nx, oldny = ny;
					if (!x_ok)
					{
						parts[i].x = remainder_p(parts[i].x-CELL+.5f, XRES-CELL*2.0f)+CELL-.5f;
						nx = (int)((float)parts[i].x+0.5f);
					}
					if (!y_ok)
					{
						parts[i].y = remainder_p(parts[i].y-CELL+.5f, YRES-CELL*2.0f)+CELL-.5f;
						ny = (int)((float)parts[i].y+0.5f);
					}

					if (!x_ok || !y_ok) //when moving from left to right stickmen might be able to fall through solid things, fix with "eval_move(t, nx+diffx, ny+diffy, NULL)" but then they die instead
					{
						//adjust stickmen legs
						playerst* stickman = NULL;
						int t = parts[i].type;
						if (t == PT_STKM)
							stickman = &player;
						else if (t == PT_STKM2)
							stickman = &player2;
						else if (t == PT_FIGH && parts[i].tmp >= 0 && parts[i].tmp < MAX_FIGHTERS)
							stickman = &fighters[parts[i].tmp];

						if (stickman)
							for (int i = 0; i < 16; i+=2)
							{
								stickman->legs[i] += (nx-oldnx);
								stickman->legs[i+1] += (ny-oldny);
								stickman->accs[i/2] *= .95f;
							}
						parts[i].vy *= .95f;
						parts[i].vx *= .95f;
					}
				}
				if (ny!=y || nx!=x)
				{
					if (pmap[y][x] && ID(pmap[y][x]) == i)
						pmap[y][x] = 0;
					else if (photons[y][x] && ID(photons[y][x]) == i)
						photons[y][x] = 0;
					if (nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)
					{
						kill_part(i);
						continue;
					}
					if (elements[t].Properties & TYPE_ENERGY)
						photons[ny][nx] = PMAP(i, t);
					else if (t)
						pmap[ny][nx] = PMAP(i, t);
				}
			}
			else if (elements[t].Properties & TYPE_ENERGY)
			{
				if (t == PT_PHOT)
				{
					if (parts[i].flags&FLAG_SKIPMOVE)
					{
						parts[i].flags &= ~FLAG_SKIPMOVE;
						continue;
					}

					if (eval_move(PT_PHOT, fin_x, fin_y, NULL))
					{
						int rt = TYP(pmap[fin_y][fin_x]);
						int lt = TYP(pmap[y][x]);
						int rt_glas = (rt == PT_GLAS) || (rt == PT_BGLA);
						int lt_glas = (lt == PT_GLAS) || (lt == PT_BGLA);
						if ((rt_glas && !lt_glas) || (lt_glas && !rt_glas))
						{
							auto gn = get_normal_interp<true>(*this, REFRACT|t, parts[i].x, parts[i].y, parts[i].vx, parts[i].vy);
							if (!gn.success) {
								kill_part(i);
								continue;
							}
							auto nrx = gn.nx;
							auto nry = gn.ny;
							auto r = get_wavelength_bin(&parts[i].ctype);
							if (r == -1 || !(parts[i].ctype&0x3FFFFFFF))
							{
								kill_part(i);
								continue;
							}
							auto nn = GLASS_IOR - GLASS_DISP*(r-30)/30.0f;
							nn *= nn;

							auto enter = rt_glas && !lt_glas;
							nrx = enter ? -nrx : nrx;
							nry = enter ? -nry : nry;
							nn = enter ? 1.0f/nn : nn;
							auto ct1 = parts[i].vx*nrx + parts[i].vy*nry;
							auto ct2 = 1.0f - (nn*nn)*(1.0f-(ct1*ct1));
							if (ct2 < 0.0f) {
								// total internal reflection
								parts[i].vx -= 2.0f*ct1*nrx;
								parts[i].vy -= 2.0f*ct1*nry;
								fin_xf = parts[i].x;
								fin_yf = parts[i].y;
								fin_x = x;
								fin_y = y;
							} else {
								// refraction
								ct2 = sqrtf(ct2);
								ct2 = ct2 - nn*ct1;
								parts[i].vx = nn*parts[i].vx + ct2*nrx;
								parts[i].vy = nn*parts[i].vy + ct2*nry;
							}
						}
					}
				}
				if (stagnant)//FLAG_STAGNANT set, was reflected on previous frame
				{
					// cast coords as int then back to float for compatibility with existing saves
					if (!do_move(i, x, y, (float)fin_x, (float)fin_y) && parts[i].type) {
						kill_part(i);
						continue;
					}
				}
				else if (!do_move(i, x, y, fin_xf, fin_yf))
				{
					if (parts[i].type == PT_NONE)
						continue;
					// reflection
					parts[i].flags |= FLAG_STAGNANT;
					if (t==PT_NEUT && rng.chance(1, 10))
					{
						kill_part(i);
						continue;
					}
					auto r = pmap[fin_y][fin_x];

					if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && !TYP(parts[ID(r)].ctype))
					{
						parts[ID(r)].ctype =  parts[i].type;
						parts[ID(r)].temp = parts[i].temp;
						parts[ID(r)].tmp2 = parts[i].life;
						parts[ID(r)].tmp3 = parts[i].tmp;
						parts[ID(r)].tmp4 = parts[i].ctype;
						kill_part(i);
						continue;
					}

					if (t == PT_PHOT)
					{
						auto mask = elements[TYP(r)].PhotonReflectWavelengths;
						if (TYP(r) == PT_LITH)
						{
							int wl_bin = parts[ID(r)].ctype / 4;
							if (wl_bin < 0) wl_bin = 0;
							if (wl_bin > 25) wl_bin = 25;
							mask = (0x1F << wl_bin);
						}
						parts[i].ctype &= mask;
					}

					auto gn = get_normal_interp<true>(*this, t, parts[i].x, parts[i].y, parts[i].vx, parts[i].vy);
					if (gn.success)
					{
						auto nrx = gn.nx;
						auto nry = gn.ny;
						if (TYP(r) == PT_CRMC)
						{
							float r = rng.between(-50, 50) * 0.01f, rx, ry, anrx, anry;
							r = r * r * r;
							rx = cosf(r); ry = sinf(r);
							anrx = rx * nrx + ry * nry;
							anry = rx * nry - ry * nrx;
							auto dp = anrx*parts[i].vx + anry*parts[i].vy;
							parts[i].vx -= 2.0f*dp*anrx;
							parts[i].vy -= 2.0f*dp*anry;
						}
						else
						{
							auto dp = nrx*parts[i].vx + nry*parts[i].vy;
							parts[i].vx -= 2.0f*dp*nrx;
							parts[i].vy -= 2.0f*dp*nry;
						}
						// leave the actual movement until next frame so that reflection of fast particles and refraction happen correctly
					}
					else
					{
						if (t!=PT_NEUT)
							kill_part(i);
						continue;
					}
					if (!(parts[i].ctype&0x3FFFFFFF) && t == PT_PHOT)
					{
						kill_part(i);
						continue;
					}
				}
			}
			else if (elements[t].Falldown==0)
			{
				// gasses and solids (but not powders)
				if (!do_move(i, x, y, fin_xf, fin_yf))
				{
					if (parts[i].type == PT_NONE)
						continue;
					// can't move there, so bounce off
					// TODO
					// TODO: Work out what previous TODO was for
					if (fin_x>x+ISTP) fin_x=x+ISTP;
					if (fin_x<x-ISTP) fin_x=x-ISTP;
					if (fin_y>y+ISTP) fin_y=y+ISTP;
					if (fin_y<y-ISTP) fin_y=y-ISTP;
					if (do_move(i, x, y, 0.25f+(float)(2*x-fin_x), 0.25f+fin_y))
					{
						parts[i].vx *= elements[t].Collision;
					}
					else if (do_move(i, x, y, 0.25f+fin_x, 0.25f+(float)(2*y-fin_y)))
					{
						parts[i].vy *= elements[t].Collision;
					}
					else
					{
						parts[i].vx *= elements[t].Collision;
						parts[i].vy *= elements[t].Collision;
					}
				}
			}
			else
			{
				// Checking stagnant is cool, but then it doesn't update when you change it later.
				if (water_equal_test && elements[t].Falldown == 2 && rng.chance(1, 200))
				{
					if (flood_water(x, y, i))
						goto movedone;
				}
				// liquids and powders
				if (!do_move(i, x, y, fin_xf, fin_yf))
				{
					if (parts[i].type == PT_NONE)
						continue;
					if (fin_x!=x && do_move(i, x, y, fin_xf, clear_yf))
					{
						parts[i].vx *= elements[t].Collision;
						parts[i].vy *= elements[t].Collision;
					}
					else if (fin_y!=y && do_move(i, x, y, clear_xf, fin_yf))
					{
						parts[i].vx *= elements[t].Collision;
						parts[i].vy *= elements[t].Collision;
					}
					else
					{
						auto r = rng.between(0, 1) * 2 - 1;// position search direction (left/right first)
						if ((clear_x!=x || clear_y!=y || nt || surround_space) &&
							(fabsf(parts[i].vx)>0.01f || fabsf(parts[i].vy)>0.01f))
						{
							// allow diagonal movement if target position is blocked
							// but no point trying this if particle is stuck in a block of identical particles
							auto dx = parts[i].vx - parts[i].vy*r;
							auto dy = parts[i].vy + parts[i].vx*r;

							auto mv = std::max(fabsf(dx), fabsf(dy));
							dx /= mv;
							dy /= mv;
							if (do_move(i, x, y, clear_xf+dx, clear_yf+dy))
							{
								parts[i].vx *= elements[t].Collision;
								parts[i].vy *= elements[t].Collision;
								goto movedone;
							}
							{
								auto swappage = dx;
								dx = dy*r;
								dy = -swappage*r;
							}
							if (do_move(i, x, y, clear_xf+dx, clear_yf+dy))
							{
								parts[i].vx *= elements[t].Collision;
								parts[i].vy *= elements[t].Collision;
								goto movedone;
							}
						}
						if (elements[t].Falldown>1 && !grav->IsEnabled() && gravityMode==GRAV_VERTICAL && parts[i].vy>fabsf(parts[i].vx))
						{
							auto s = 0;
							// stagnant is true if FLAG_STAGNANT was set for this particle in previous frame
							int rt;
							if (!stagnant || nt) //nt is if there is an something else besides the current particle type, around the particle
								rt = 30;//slight less water lag, although it changes how it moves a lot
							else
								rt = 10;

							if (t==PT_GEL)
								rt = int(parts[i].tmp*0.20f+5.0f);

							auto nx = -1, ny = -1;
							for (auto j=clear_x+r; j>=0 && j>=clear_x-rt && j<clear_x+rt && j<XRES; j+=r)
							{
								if ((TYP(pmap[fin_y][j])!=t || bmap[fin_y/CELL][j/CELL])
									&& (s=do_move(i, x, y, (float)j, fin_yf)))
								{
									nx = (int)(parts[i].x+0.5f);
									ny = (int)(parts[i].y+0.5f);
									break;
								}
								if (fin_y!=clear_y && (TYP(pmap[clear_y][j])!=t || bmap[clear_y/CELL][j/CELL])
									&& (s=do_move(i, x, y, (float)j, clear_yf)))
								{
									nx = (int)(parts[i].x+0.5f);
									ny = (int)(parts[i].y+0.5f);
									break;
								}
								if (TYP(pmap[clear_y][j])!=t || (bmap[clear_y/CELL][j/CELL] && bmap[clear_y/CELL][j/CELL]!=WL_STREAM))
									break;
							}

							r = (parts[i].vy>0) ? 1 : -1;

							if (s==1)
								for (auto j=ny+r; j>=0 && j<YRES && j>=ny-rt && j<ny+rt; j+=r)
								{
									if ((TYP(pmap[j][nx])!=t || bmap[j/CELL][nx/CELL]) && do_move(i, nx, ny, (float)nx, (float)j))
										break;
									if (TYP(pmap[j][nx])!=t || (bmap[j/CELL][nx/CELL] && bmap[j/CELL][nx/CELL]!=WL_STREAM))
										break;
								}
							else if (s==-1) {} // particle is out of bounds
							else if ((clear_x!=x||clear_y!=y) && do_move(i, x, y, clear_xf, clear_yf)) {}
							else parts[i].flags |= FLAG_STAGNANT;
							parts[i].vx *= elements[t].Collision;
							parts[i].vy *= elements[t].Collision;
						}
						else if (elements[t].Falldown>1 && fabsf(pGravX*parts[i].vx+pGravY*parts[i].vy)>fabsf(pGravY*parts[i].vx-pGravX*parts[i].vy))
						{
							float nxf, nyf, prev_pGravX, prev_pGravY, ptGrav = elements[t].Gravity;
							auto s = 0;
							// stagnant is true if FLAG_STAGNANT was set for this particle in previous frame
							// nt is if there is something else besides the current particle type around the particle
							// 30 gives slightly less water lag, although it changes how it moves a lot
							auto rt = (!stagnant || nt) ? 30 : 10;

							// clear_xf, clear_yf is the last known position that the particle should almost certainly be able to move to
							nxf = clear_xf;
							nyf = clear_yf;
							auto nx = clear_x;
							auto ny = clear_y;
							// Look for spaces to move horizontally (perpendicular to gravity direction), keep going until a space is found or the number of positions examined = rt
							for (auto j=0;j<rt;j++)
							{
								// Calculate overall gravity direction
								GetGravityField(nx, ny, ptGrav, 1.0f, pGravX, pGravY);
								// Scale gravity vector so that the largest component is 1 pixel
								auto mv = std::max(fabsf(pGravX), fabsf(pGravY));
								if (mv<0.0001f) break;
								pGravX /= mv;
								pGravY /= mv;
								// Move 1 pixel perpendicularly to gravity
								// r is +1/-1, to try moving left or right at random
								if (j)
								{
									// Not quite the gravity direction
									// Gravity direction + last change in gravity direction
									// This makes liquid movement a bit less frothy, particularly for balls of liquid in radial gravity. With radial gravity, instead of just moving along a tangent, the attempted movement will follow the curvature a bit better.
									nxf += r*(pGravY*2.0f-prev_pGravY);
									nyf += -r*(pGravX*2.0f-prev_pGravX);
								}
								else
								{
									nxf += r*pGravY;
									nyf += -r*pGravX;
								}
								prev_pGravX = pGravX;
								prev_pGravY = pGravY;
								// Check whether movement is allowed
								nx = (int)(nxf+0.5f);
								ny = (int)(nyf+0.5f);
								if (nx<0 || ny<0 || nx>=XRES || ny >=YRES)
									break;
								if (TYP(pmap[ny][nx])!=t || bmap[ny/CELL][nx/CELL])
								{
									s = do_move(i, x, y, nxf, nyf);
									if (s)
									{
										// Movement was successful
										nx = (int)(parts[i].x+0.5f);
										ny = (int)(parts[i].y+0.5f);
										break;
									}
									// A particle of a different type, or a wall, was found. Stop trying to move any further horizontally unless the wall should be completely invisible to particles.
									if (TYP(pmap[ny][nx])!=t || bmap[ny/CELL][nx/CELL]!=WL_STREAM)
										break;
								}
							}
							if (s==1)
							{
								// The particle managed to move horizontally, now try to move vertically (parallel to gravity direction)
								// Keep going until the particle is blocked (by something that isn't the same element) or the number of positions examined = rt
								clear_x = nx;
								clear_y = ny;
								for (auto j=0;j<rt;j++)
								{
									// Calculate overall gravity direction
									GetGravityField(nx, ny, ptGrav, 1.0f, pGravX, pGravY);
									// Scale gravity vector so that the largest component is 1 pixel
									auto mv = std::max(fabsf(pGravX), fabsf(pGravY));
									if (mv<0.0001f) break;
									pGravX /= mv;
									pGravY /= mv;
									// Move 1 pixel in the direction of gravity
									nxf += pGravX;
									nyf += pGravY;
									nx = (int)(nxf+0.5f);
									ny = (int)(nyf+0.5f);
									if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
										break;
									// If the space is anything except the same element (a wall, empty space, or occupied by a particle of a different element), try to move into it
									if (TYP(pmap[ny][nx])!=t || bmap[ny/CELL][nx/CELL])
									{
										s = do_move(i, clear_x, clear_y, nxf, nyf);
										if (s || TYP(pmap[ny][nx])!=t || bmap[ny/CELL][nx/CELL]!=WL_STREAM)
											break; // found the edge of the liquid and movement into it succeeded, so stop moving down
									}
								}
							}
							else if (s==-1) {} // particle is out of bounds
							else if ((clear_x!=x||clear_y!=y) && do_move(i, x, y, clear_xf, clear_yf)) {} // try moving to the last clear position
							else parts[i].flags |= FLAG_STAGNANT;
							parts[i].vx *= elements[t].Collision;
							parts[i].vy *= elements[t].Collision;
						}
						else
						{
							// if interpolation was done, try moving to last clear position
							if ((clear_x!=x||clear_y!=y) && do_move(i, x, y, clear_xf, clear_yf)) {}
							else parts[i].flags |= FLAG_STAGNANT;
							parts[i].vx *= elements[t].Collision;
							parts[i].vy *= elements[t].Collision;
						}
					}
				}
			}
movedone:
			continue;
		}
	}

	//'f' was pressed (single frame)
	if (framerender)
	{
		framerender--;
	}
}

void Simulation::RecalcFreeParticles(bool do_life_dec)
{
	int x, y, t;
	int lastPartUsed = 0;
	int lastPartUnused = -1;

	memset(pmap, 0, sizeof(pmap));
	memset(pmap_count, 0, sizeof(pmap_count));
	memset(photons, 0, sizeof(photons));

	NUM_PARTS = 0;
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	//the particle loop that resets the pmap/photon maps every frame, to update them.
	for (int i = 0; i <= parts_lastActiveIndex; i++)
	{
		if (parts[i].type)
		{
			t = parts[i].type;
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			bool inBounds = false;
			if (x>=0 && y>=0 && x<XRES && y<YRES)
			{
				if (elements[t].Properties & TYPE_ENERGY)
					photons[y][x] = PMAP(i, t);
				else
				{
					// Particles are sometimes allowed to go inside INVS and FILT
					// To make particles collide correctly when inside these elements, these elements must not overwrite an existing pmap entry from particles inside them
					if (!pmap[y][x] || (t!=PT_INVIS && t!= PT_FILT))
						pmap[y][x] = PMAP(i, t);
					// (there are a few exceptions, including energy particles - currently no limit on stacking those)
					if (t!=PT_THDR && t!=PT_EMBR && t!=PT_FIGH && t!=PT_PLSM)
						pmap_count[y][x]++;
				}
				inBounds = true;
			}
			lastPartUsed = i;
			NUM_PARTS ++;

			if (elementRecount && t >= 0 && t < PT_NUM && elements[t].Enabled)
				elementCount[t]++;

			//decrease particle life
			if (do_life_dec && (!sys_pause || framerender))
			{
				if (t<0 || t>=PT_NUM || !elements[t].Enabled)
				{
					kill_part(i);
					continue;
				}

				unsigned int elem_properties = elements[t].Properties;
				if (parts[i].life>0 && (elem_properties&PROP_LIFE_DEC) && !(inBounds && bmap[y/CELL][x/CELL] == WL_STASIS && emap[y/CELL][x/CELL]<8))
				{
					// automatically decrease life
					parts[i].life--;
					if (parts[i].life<=0 && (elem_properties&(PROP_LIFE_KILL_DEC|PROP_LIFE_KILL)))
					{
						// kill on change to no life
						kill_part(i);
						continue;
					}
				}
				else if (parts[i].life<=0 && (elem_properties&PROP_LIFE_KILL) && !(inBounds && bmap[y/CELL][x/CELL] == WL_STASIS && emap[y/CELL][x/CELL]<8))
				{
					// kill if no life
					kill_part(i);
					continue;
				}
			}
		}
		else
		{
			if (lastPartUnused<0) pfree = i;
			else parts[lastPartUnused].life = i;
			lastPartUnused = i;
		}
	}
	if (lastPartUnused == -1)
	{
		pfree = (parts_lastActiveIndex>=(NPART-1)) ? -1 : parts_lastActiveIndex+1;
	}
	else
	{
		parts[lastPartUnused].life = (parts_lastActiveIndex>=(NPART-1)) ? -1 : parts_lastActiveIndex+1;
	}
	parts_lastActiveIndex = lastPartUsed;
	if (elementRecount)
		elementRecount = false;
}

void Simulation::SimulateGoL()
{
	auto &builtinGol = SimulationData::builtinGol;
	CGOL = 0;
	for (int i = 0; i <= parts_lastActiveIndex; ++i)
	{
		auto &part = parts[i];
		if (part.type != PT_LIFE)
		{
			continue;
		}
		auto x = int(part.x + 0.5f);
		auto y = int(part.y + 0.5f);
		if (x < CELL || y < CELL || x >= XRES - CELL || y >= YRES - CELL)
		{
			continue;
		}
		unsigned int golnum = part.ctype;
		unsigned int ruleset = golnum;
		if (golnum < NGOL)
		{
			ruleset = builtinGol[golnum].ruleset;
			golnum += 1;
		}
		if (part.tmp2 == int((ruleset >> 17) & 0xF) + 1)
		{
			for (int yy = -1; yy <= 1; ++yy)
			{
				for (int xx = -1; xx <= 1; ++xx)
				{
					if (xx || yy)
					{
						// * Calculate address of the neighbourList, taking wraparound
						//   into account. The fact that the GOL space is 2 CELL's worth
						//   narrower in both dimensions than the simulation area makes
						//   this a bit awkward.
						int ax = ((x + xx + XRES - 3 * CELL) % (XRES - 2 * CELL)) + CELL;
						int ay = ((y + yy + YRES - 3 * CELL) % (YRES - 2 * CELL)) + CELL;
						if (pmap[ay][ax] && TYP(pmap[ay][ax]) != PT_LIFE)
						{
							continue;
						}
						unsigned int (&neighbourList)[5] = gol[ay][ax];
						// * Bump overall neighbour counter (bits 30..28) for the entire list.
						neighbourList[0] += 1U << 28;
						for (int l = 0; l < 5; ++l)
						{
							auto neighbourRuleset = neighbourList[l] & 0x001FFFFFU;
							if (neighbourRuleset == golnum)
							{
								// * Bump population counter (bits 23..21) of the
								//   same kind of cell.
								neighbourList[l] += 1U << 21;
								break;
							}
							if (neighbourRuleset == 0)
							{
								// * Add the new kind of cell to the population. Both counters
								//   have a bias of -1, so they're intentionally initialised
								//   to 0 instead of 1 here. This is all so they can both
								//   fit in 3 bits.
								neighbourList[l] = ((yy & 3) << 26) | ((xx & 3) << 24) | golnum;
								break;
							}
							// * If after 5 iterations the cell still hasn't contributed
							//   to a list entry, it's surely a 6th kind of cell, meaning
							//   there could be at most 3 of it in the neighbourhood,
							//   as there are already 5 other kinds of cells present in
							//   the list. This in turn means that it couldn't possibly
							//   win the population ratio-based contest later on.
						}
					}
				}
			}
		}
		else
		{
			if (!(bmap[y / CELL][x / CELL] == WL_STASIS && emap[y / CELL][x / CELL] < 8))
			{
				part.tmp2 -= 1;
			}
		}
	}
	for (int y = CELL; y < YRES - CELL; ++y)
	{
		for (int x = CELL; x < XRES - CELL; ++x)
		{
			int r = pmap[y][x];
			if (r && TYP(r) != PT_LIFE)
			{
				continue;
			}
			unsigned int (&neighbourList)[5] = gol[y][x];
			auto nl0 = neighbourList[0];
			if (r || nl0)
			{
				// * Get overall neighbour count (bits 30..28).
				unsigned int neighbours = nl0 ? ((nl0 >> 28) & 7) + 1 : 0;
				if (!(bmap[y / CELL][x / CELL] == WL_STASIS && emap[y / CELL][x / CELL] < 8))
				{
					if (r)
					{
						auto &part = parts[ID(r)];
						unsigned int ruleset = part.ctype;
						if (ruleset < NGOL)
						{
							ruleset = builtinGol[ruleset].ruleset;
						}
						if (!((ruleset >> neighbours) & 1) && part.tmp2 == int(ruleset >> 17) + 1)
						{
							// * Start death sequence.
							part.tmp2 -= 1;
						}
					}
					else
					{
						unsigned int golnumToCreate = 0xFFFFFFFFU;
						unsigned int createFromEntry = 0U;
						unsigned int majority = neighbours / 2 + neighbours % 2;
						for (int l = 0; l < 5; ++l)
						{
							auto golnum = neighbourList[l] & 0x001FFFFFU;
							if (!golnum)
							{
								break;
							}
							auto ruleset = golnum;
							if (golnum - 1 < NGOL)
							{
								ruleset = builtinGol[golnum - 1].ruleset;
								golnum -= 1;
							}
							if ((ruleset >> (neighbours + 8)) & 1 && ((neighbourList[l] >> 21) & 7) + 1 >= majority && golnum < golnumToCreate)
							{
								golnumToCreate = golnum;
								createFromEntry = neighbourList[l];
							}
						}
						if (golnumToCreate != 0xFFFFFFFFU)
						{
							// * 0x200000: No need to look for colours, they'll be set later anyway.
							int i = create_part(-1, x, y, PT_LIFE, golnumToCreate | 0x200000);
							if (i >= 0)
							{
								int xx = (createFromEntry >> 24) & 3;
								int yy = (createFromEntry >> 26) & 3;
								if (xx == 3) xx = -1;
								if (yy == 3) yy = -1;
								int ax = ((x - xx + XRES - 3 * CELL) % (XRES - 2 * CELL)) + CELL;
								int ay = ((y - yy + YRES - 3 * CELL) % (YRES - 2 * CELL)) + CELL;
								auto &sample = parts[ID(pmap[ay][ax])];
								parts[i].dcolour = sample.dcolour;
								parts[i].tmp = sample.tmp;
							}
						}
					}
				}
				for (int l = 0; l < 5 && neighbourList[l]; ++l)
				{
					neighbourList[l] = 0;
				}
			}
		}
	}
	for (int y = CELL; y < YRES - CELL; ++y)
	{
		for (int x = CELL; x < XRES - CELL; ++x)
		{
			int r = pmap[y][x];
			if (r && TYP(r) == PT_LIFE && parts[ID(r)].tmp2 <= 0)
			{
				kill_part(ID(r));
			}
		}
	}
}

void Simulation::CheckStacking()
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	bool excessive_stacking_found = false;
	force_stacking_check = false;
	for (int y = 0; y < YRES; y++)
	{
		for (int x = 0; x < XRES; x++)
		{
			// Use a threshold, since some particle stacking can be normal (e.g. BIZR + FILT)
			// Setting pmap_count[y][x] > NPART means BHOL will form in that spot
			if (pmap_count[y][x]>5)
			{
				if (bmap[y/CELL][x/CELL]==WL_EHOLE)
				{
					// Allow more stacking in E-hole
					if (pmap_count[y][x]>1500)
					{
						pmap_count[y][x] = pmap_count[y][x] + NPART;
						excessive_stacking_found = 1;
					}
				}
				else if (pmap_count[y][x]>1500 || (unsigned int)rng.between(0, 1599) <= (pmap_count[y][x]+100))
				{
					pmap_count[y][x] = pmap_count[y][x] + NPART;
					excessive_stacking_found = true;
				}
			}
		}
	}
	if (excessive_stacking_found)
	{
		for (int i = 0; i <= parts_lastActiveIndex; i++)
		{
			if (parts[i].type)
			{
				int t = parts[i].type;
				int x = (int)(parts[i].x+0.5f);
				int y = (int)(parts[i].y+0.5f);
				if (x>=0 && y>=0 && x<XRES && y<YRES && !(elements[t].Properties&TYPE_ENERGY))
				{
					if (pmap_count[y][x]>=NPART)
					{
						if (pmap_count[y][x]>NPART)
						{
							create_part(i, x, y, PT_NBHL);
							parts[i].temp = MAX_TEMP;
							parts[i].tmp = pmap_count[y][x]-NPART;//strength of grav field
							if (parts[i].tmp>51200) parts[i].tmp = 51200;
							pmap_count[y][x] = NPART;
						}
						else
						{
							kill_part(i);
						}
					}
				}
			}
		}
	}
}

//updates pmap, gol, and some other simulation stuff (but not particles)
void Simulation::BeforeSim()
{
	if (!sys_pause||framerender)
	{
		air->update_air();

		if(aheat_enable)
			air->update_airh();

		if(grav->IsEnabled())
		{
			grav->gravity_update_async();

			//Get updated buffer pointers for gravity
			gravx = &grav->gravx[0];
			gravy = &grav->gravy[0];
			gravp = &grav->gravp[0];
			gravmap = &grav->gravmap[0];
		}
		if(emp_decor>0)
			emp_decor -= emp_decor/25+2;
		if(emp_decor < 0)
			emp_decor = 0;
		etrd_count_valid = false;
		etrd_life0_count = 0;

		currentTick++;

		elementRecount |= !(currentTick%180);
		if (elementRecount)
			std::fill(elementCount, elementCount+PT_NUM, 0);
	}
	sandcolour_interface = (int)(20.0f*sin((float)sandcolour_frame*(TPT_PI_FLT/180.0f)));
	sandcolour_frame = (sandcolour_frame+1)%360;
	sandcolour = (int)(20.0f*sin((float)(frameCount)*(TPT_PI_FLT/180.0f)));

	if (gravWallChanged)
	{
		grav->gravity_mask();
		gravWallChanged = false;
	}

	if (debug_nextToUpdate == 0)
		RecalcFreeParticles(true);

	if (!sys_pause || framerender)
	{
		// decrease wall conduction, make walls block air and ambient heat
		int x, y;
		for (y = 0; y < YCELLS; y++)
		{
			for (x = 0; x < XCELLS; x++)
			{
				if (emap[y][x])
					emap[y][x] --;
				air->bmap_blockair[y][x] = (bmap[y][x]==WL_WALL || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_BLOCKAIR || (bmap[y][x]==WL_EWALL && !emap[y][x]));
				air->bmap_blockairh[y][x] = (bmap[y][x]==WL_WALL || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_BLOCKAIR || bmap[y][x]==WL_GRAV || (bmap[y][x]==WL_EWALL && !emap[y][x])) ? 0x8:0;
			}
		}

		// check for stacking and create BHOL if found
		if (force_stacking_check || rng.chance(1, 10))
		{
			CheckStacking();
		}

		// LOVE and LOLZ element handling
		if (elementCount[PT_LOVE] > 0 || elementCount[PT_LOLZ] > 0)
		{
			int nx, nnx, ny, nny, r, rt;
			for (ny=0; ny<YRES-4; ny++)
			{
				for (nx=0; nx<XRES-4; nx++)
				{
					r=pmap[ny][nx];
					if (!r)
					{
						continue;
					}
					else if ((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&(parts[ID(r)].type==PT_LOVE||parts[ID(r)].type==PT_LOLZ))
						kill_part(ID(r));
					else if (parts[ID(r)].type==PT_LOVE)
					{
						Element_LOVE_love[nx/9][ny/9] = 1;
					}
					else if (parts[ID(r)].type==PT_LOLZ)
					{
						Element_LOLZ_lolz[nx/9][ny/9] = 1;
					}
				}
			}
			for (nx=9; nx<=XRES-18; nx++)
			{
				for (ny=9; ny<=YRES-7; ny++)
				{
					if (Element_LOVE_love[nx/9][ny/9]==1)
					{
						for ( nnx=0; nnx<9; nnx++)
							for ( nny=0; nny<9; nny++)
							{
								if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
								{
									rt=pmap[ny+nny][nx+nnx];
									if (!rt&&Element_LOVE_RuleTable[nnx][nny]==1)
										create_part(-1,nx+nnx,ny+nny,PT_LOVE);
									else if (!rt)
										continue;
									else if (parts[ID(rt)].type==PT_LOVE&&Element_LOVE_RuleTable[nnx][nny]==0)
										kill_part(ID(rt));
								}
							}
					}
					Element_LOVE_love[nx/9][ny/9]=0;
					if (Element_LOLZ_lolz[nx/9][ny/9]==1)
					{
						for ( nnx=0; nnx<9; nnx++)
							for ( nny=0; nny<9; nny++)
							{
								if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
								{
									rt=pmap[ny+nny][nx+nnx];
									if (!rt&&Element_LOLZ_RuleTable[nny][nnx]==1)
										create_part(-1,nx+nnx,ny+nny,PT_LOLZ);
									else if (!rt)
										continue;
									else if (parts[ID(rt)].type==PT_LOLZ&&Element_LOLZ_RuleTable[nny][nnx]==0)
										kill_part(ID(rt));

								}
							}
					}
					Element_LOLZ_lolz[nx/9][ny/9]=0;
				}
			}
		}

		// make WIRE work
		if(elementCount[PT_WIRE] > 0)
		{
			for (int nx = 0; nx < XRES; nx++)
			{
				for (int ny = 0; ny < YRES; ny++)
				{
					int r = pmap[ny][nx];
					if (!r)
						continue;
					if(parts[ID(r)].type == PT_WIRE)
						parts[ID(r)].tmp = parts[ID(r)].ctype;
				}
			}
		}

		// update PPIP tmp?
		if (Element_PPIP_ppip_changed)
		{
			for (int i = 0; i <= parts_lastActiveIndex; i++)
			{
				if (parts[i].type==PT_PPIP)
				{
					parts[i].tmp |= (parts[i].tmp&0xE0000000)>>3;
					parts[i].tmp &= ~0xE0000000;
				}
			}
			Element_PPIP_ppip_changed = 0;
		}

		// Simulate GoL
		// GSPEED is frames per generation
		if (elementCount[PT_LIFE]>0 && ++CGOL>=GSPEED)
		{
			SimulateGoL();
		}

		// wifi channel reseting
		if (ISWIRE > 0)
		{
			for (int q = 0; q < (int)(MAX_TEMP-73.15f)/100+2; q++)
			{
				wireless[q][0] = wireless[q][1];
				wireless[q][1] = 0;
			}
			ISWIRE--;
		}

		// spawn STKM and STK2
		if (!player.spwn && player.spawnID >= 0)
			create_part(-1, (int)parts[player.spawnID].x, (int)parts[player.spawnID].y, PT_STKM);
		if (!player2.spwn && player2.spawnID >= 0)
			create_part(-1, (int)parts[player2.spawnID].x, (int)parts[player2.spawnID].y, PT_STKM2);

		// particle update happens right after this function (called separately)
	}
}

void Simulation::AfterSim()
{
	debug_mostRecentlyUpdated = -1;

	if (emp_trigger_count)
	{
		// pitiful attempt at trying to keep code relating to a given element in the same file
		Element_EMP_Trigger(this, emp_trigger_count);
		emp_trigger_count = 0;
	}

	frameCount += 1;
}

Simulation::~Simulation() = default;

Simulation::Simulation():
	replaceModeSelected(0),
	replaceModeFlags(0),
	debug_nextToUpdate(0),
	ISWIRE(0),
	force_stacking_check(false),
	emp_decor(0),
	emp_trigger_count(0),
	etrd_count_valid(false),
	etrd_life0_count(0),
	lightningRecreate(0),
	gravWallChanged(false),
	CGOL(0),
	GSPEED(1),
	edgeMode(EDGE_VOID),
	gravityMode(GRAV_VERTICAL),
	customGravityX(0),
	customGravityY(0),
	legacy_enable(0),
	aheat_enable(0),
	water_equal_test(0),
	sys_pause(0),
	framerender(0),
	pretty_powder(0),
	sandcolour_frame(0),
	deco_space(0)
{
	int tportal_rx[] = {-1, 0, 1, 1, 1, 0,-1,-1};
	int tportal_ry[] = {-1,-1,-1, 0, 1, 1, 1, 0};

	memcpy(portal_rx, tportal_rx, sizeof(tportal_rx));
	memcpy(portal_ry, tportal_ry, sizeof(tportal_ry));

	currentTick = 0;
	std::fill(elementCount, elementCount+PT_NUM, 0);
	elementRecount = true;

	//Create and attach gravity simulation
	grav = Gravity::Create();
	//Give air sim references to our data
	grav->bmap = bmap;
	//Gravity sim gives us maps to use
	gravx = &grav->gravx[0];
	gravy = &grav->gravy[0];
	gravp = &grav->gravp[0];
	gravmap = &grav->gravmap[0];

	//Create and attach air simulation
	air = std::make_unique<Air>(*this);
	//Give air sim references to our data
	air->bmap = bmap;
	air->emap = emap;
	air->fvx = fvx;
	air->fvy = fvy;
	//Air sim gives us maps to use
	vx = air->vx;
	vy = air->vy;
	pv = air->pv;
	hv = air->hv;

	player.comm = 0;
	player2.comm = 0;

	clear_sim();

	grav->gravity_mask();
}

constexpr size_t ce_log2(size_t n)
{
	return ((n < 2) ? 1 : 1 + ce_log2(n / 2));
}
static_assert(PMAPBITS <= 16, "PMAPBITS is too large");
// * This will technically fail in some cases where (XRES * YRES) << PMAPBITS would
//   fit in 31 bits but multiplication is evil and wraps around without you knowing it.
// * Whoever runs into a problem with this (e.g. with XRES = 612, YRES = 384 and
//   PMAPBITS = 13) should just remove the check and take responsibility otherwise.
static_assert(ce_log2(XRES) + ce_log2(YRES) + PMAPBITS <= 31, "not enough space in pmap");
