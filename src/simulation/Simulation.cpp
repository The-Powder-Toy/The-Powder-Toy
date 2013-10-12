//#include <cstdlib>
#include <cmath>
#include <math.h>
#if !defined(_MSC_VER)
#include <strings.h>
#else
#include <windows.h>
#endif
#include "Config.h"
#include "Simulation.h"
#include "Elements.h"
//#include "ElementFunctions.h"
#include "Air.h"
#include "Gravity.h"
#include "elements/Element.h"

//#include "graphics/Renderer.h"
//#include "graphics/Graphics.h"
#include "Misc.h"
#include "ToolClasses.h"
#include "gui/game/Brush.h"
#include "client/GameSave.h"
#include "Sample.h"
#include "Snapshot.h"
//#include "StorageClasses.h"

#ifdef LUACONSOLE
#include "cat/LuaScriptInterface.h"
#include "cat/LuaScriptHelper.h"
#endif

int Simulation::Load(GameSave * save)
{
	return Load(0, 0, save);
}

int Simulation::Load(int fullX, int fullY, GameSave * save)
{
	int blockX, blockY, x, y, r;

	if(!save) return 1;
	save->Expand();

	//Align to blockMap
	blockX = fullX/CELL;
	blockY = fullY/CELL;
	fullX = blockX*CELL;
	fullY = blockY*CELL;

	int partMap[PT_NUM];
	for(int i = 0; i < PT_NUM; i++)
	{
		partMap[i] = i;
	}
	if(save->palette.size())
	{
		for(std::vector<GameSave::PaletteItem>::iterator iter = save->palette.begin(), end = save->palette.end(); iter != end; ++iter)
		{
			GameSave::PaletteItem pi = *iter;
			if(pi.second >= DEFAULT_PT_NUM && pi.second < PT_NUM)
			{
				int myId = 0;//pi.second;
				for(int i = 0; i < PT_NUM; i++)
				{
					if(elements[i].Enabled && elements[i].Identifier == pi.first)
						myId = i;
				}
				partMap[pi.second] = myId;
			}
		}
	}

	int i;
	for(int n = 0; n < NPART && n < save->particlesCount; n++)
	{
		Particle tempPart = save->particles[n];
		tempPart.x += (float)fullX;
		tempPart.y += (float)fullY;
		x = int(tempPart.x + 0.5f);
		y = int(tempPart.y + 0.5f);

		if(tempPart.type >= 0 && tempPart.type < PT_NUM)
			tempPart.type = partMap[tempPart.type];

		if ((player.spwn == 1 && tempPart.type==PT_STKM) || (player2.spwn == 1 && tempPart.type==PT_STKM2))
			continue;
		if (!elements[tempPart.type].Enabled)
			continue;

		if(r = pmap[y][x])
		{
			//Replace existing
			parts[r>>8] = tempPart;
			i = r>>8;
			pmap[y][x] = 0;
			elementCount[parts[r>>8].type]--;
			elementCount[tempPart.type]++;
		}
		else
		{
			//Allocate new particle
			if (pfree == -1)
				break;
			i = pfree;
			pfree = parts[i].life;
			if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;
			parts[i] = tempPart;

			elementCount[tempPart.type]++;
		}

		if (parts[i].type == PT_STKM)
		{
			Element_STKM::STKM_init_legs(this, &player, i);
			player.spwn = 1;
			player.elem = PT_DUST;
			player.rocketBoots = false;
		}
		else if (parts[i].type == PT_STKM2)
		{
			Element_STKM::STKM_init_legs(this, &player2, i);
			player2.spwn = 1;
			player2.elem = PT_DUST;
			player2.rocketBoots = false;
		}
		else if (parts[i].type == PT_FIGH)
		{
			//TODO: 100 should be replaced with a macro
			for(int fcount = 0; fcount < 100; fcount++)
			{
				if(!fighters[fcount].spwn)
				{
					fighcount++;
					//currentPart.tmp = fcount;
					parts[i].tmp = fcount;
					Element_STKM::STKM_init_legs(this, &(fighters[fcount]), i);
					fighters[fcount].spwn = 1;
					fighters[fcount].elem = PT_DUST;
					break;
				}
			}
		}
	}
	parts_lastActiveIndex = NPART-1;
	force_stacking_check = 1;
	Element_PPIP::ppip_changed = 1;
	for(int i = 0; i < save->signs.size() && signs.size() < MAXSIGNS; i++)
	{
		if (save->signs[i].text[0])
		{
			sign tempSign = save->signs[i];
			tempSign.x += fullX;
			tempSign.y += fullY;
			signs.push_back(tempSign);
		}
	}
	for(int saveBlockX = 0; saveBlockX < save->blockWidth; saveBlockX++)
	{
		for(int saveBlockY = 0; saveBlockY < save->blockHeight; saveBlockY++)
		{
			if(save->blockMap[saveBlockY][saveBlockX])
			{
				bmap[saveBlockY+blockY][saveBlockX+blockX] = save->blockMap[saveBlockY][saveBlockX];
				fvx[saveBlockY+blockY][saveBlockX+blockX] = save->fanVelX[saveBlockY][saveBlockX];
				fvy[saveBlockY+blockY][saveBlockX+blockX] = save->fanVelY[saveBlockY][saveBlockX];
			}
		}
	}

	gravWallChanged = true;
	CalculateBlockerWall();

	return 0;
}

GameSave * Simulation::Save()
{
	return Save(0, 0, XRES-1, YRES-1);
}

GameSave * Simulation::Save(int fullX, int fullY, int fullX2, int fullY2)
{
	int blockX, blockY, blockX2, blockY2, fullW, fullH, blockW, blockH;
	//Normalise incoming coords
	int swapTemp;
	if(fullY>fullY2)
	{
		swapTemp = fullY;
		fullY = fullY2;
		fullY2 = swapTemp;
	}
	if(fullX>fullX2)
	{
		swapTemp = fullX;
		fullX = fullX2;
		fullX2 = swapTemp;
	}

	//Align coords to blockMap
	blockX = fullX/CELL;
	blockY = fullY/CELL;

	blockX2 = (fullX2+CELL)/CELL;
	blockY2 = (fullY2+CELL)/CELL;

	//fullX = blockX*CELL;
	//fullY = blockY*CELL;

	//fullX2 = blockX2*CELL;
	//fullY2 = blockY2*CELL;

	blockW = blockX2-blockX;
	blockH = blockY2-blockY;
	fullW = fullX2-fullX;
	fullH = fullY2-fullY;

	GameSave * newSave = new GameSave(blockW, blockH);
	
	int storedParts = 0;
	int elementCount[PT_NUM];
	std::fill(elementCount, elementCount+PT_NUM, 0);
	for(int i = 0; i < NPART; i++)
	{
		int x, y;
		x = int(parts[i].x + 0.5f);
		y = int(parts[i].y + 0.5f);
		if(parts[i].type && x >= fullX && y >= fullY && x <= fullX2 && y <= fullY2)
		{
			Particle tempPart = parts[i];
			tempPart.x -= blockX*CELL;
			tempPart.y -= blockY*CELL;
			if(elements[tempPart.type].Enabled)
			{
				*newSave << tempPart;
				storedParts++;
				elementCount[tempPart.type]++;
			}
		}
	}

	if(storedParts)
	{
		for(int i = DEFAULT_PT_NUM; i < PT_NUM; i++)
		{
			if(elements[i].Enabled && elementCount[i])
			{
				newSave->palette.push_back(GameSave::PaletteItem(elements[i].Identifier, i));
			}
		}
	}
	
	for(int i = 0; i < MAXSIGNS && i < signs.size(); i++)
	{
		if(signs[i].text.length() && signs[i].x >= fullX && signs[i].y >= fullY && signs[i].x <= fullX2 && signs[i].y <= fullY2)
		{
			sign tempSign = signs[i];
			tempSign.x -= blockX*CELL;
			tempSign.y -= blockY*CELL;
			*newSave << tempSign;
		}
	}
	
	for(int saveBlockX = 0; saveBlockX < newSave->blockWidth; saveBlockX++)
	{
		for(int saveBlockY = 0; saveBlockY < newSave->blockHeight; saveBlockY++)
		{
			if(bmap[saveBlockY+blockY][saveBlockX+blockX])
			{
				newSave->blockMap[saveBlockY][saveBlockX] = bmap[saveBlockY+blockY][saveBlockX+blockX];
				newSave->fanVelX[saveBlockY][saveBlockX] = fvx[saveBlockY+blockY][saveBlockX+blockX];
				newSave->fanVelY[saveBlockY][saveBlockX] = fvy[saveBlockY+blockY][saveBlockX+blockX];
			}
		}
	}

	return newSave;
}

Snapshot * Simulation::CreateSnapshot()
{
	Snapshot * snap = new Snapshot();
	snap->AirPressure.insert(snap->AirPressure.begin(), &pv[0][0], &pv[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->AirVelocityX.insert(snap->AirVelocityX.begin(), &vx[0][0], &vx[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->AirVelocityY.insert(snap->AirVelocityY.begin(), &vy[0][0], &vy[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->AmbientHeat.insert(snap->AmbientHeat.begin(), &hv[0][0], &hv[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->Particles.insert(snap->Particles.begin(), parts, parts+NPART);
	snap->PortalParticles.insert(snap->PortalParticles.begin(), &portalp[0][0][0][0], &portalp[2-1][CHANNELS-1][8-1][80-1]);
	snap->WirelessData.insert(snap->WirelessData.begin(), &wireless[0][0], &wireless[BLOCKER_CHANNELS-1][CHANNELS-1]);
	snap->GravVelocityX.insert(snap->GravVelocityX.begin(), gravx, gravx+((XRES/CELL)*(YRES/CELL)));
	snap->GravVelocityY.insert(snap->GravVelocityY.begin(), gravy, gravy+((XRES/CELL)*(YRES/CELL)));
	snap->GravValue.insert(snap->GravValue.begin(), gravp, gravp+((XRES/CELL)*(YRES/CELL)));
	snap->GravMap.insert(snap->GravMap.begin(), gravmap, gravmap+((XRES/CELL)*(YRES/CELL)));
	snap->BlockMap.insert(snap->BlockMap.begin(), &bmap[0][0], &bmap[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->ElecMap.insert(snap->ElecMap.begin(), &emap[0][0], &emap[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->FanVelocityX.insert(snap->FanVelocityX.begin(), &fvx[0][0], &fvx[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->FanVelocityY.insert(snap->FanVelocityY.begin(), &fvy[0][0], &fvy[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->stickmen.push_back(player2);
	snap->stickmen.push_back(player);
	snap->stickmen.insert(snap->stickmen.begin(), &fighters[0], &fighters[255]);
	snap->signs = signs;
	return snap;
}

void Simulation::Restore(const Snapshot & snap)
{
	parts_lastActiveIndex = NPART-1; 

	for(int i = 0; i<NPART; i++)
		elementCount[i] = 0;

	std::copy(snap.AirPressure.begin(), snap.AirPressure.end(), &pv[0][0]);
	std::copy(snap.AirVelocityX.begin(), snap.AirVelocityX.end(), &vx[0][0]);
	std::copy(snap.AirVelocityY.begin(), snap.AirVelocityY.end(), &vy[0][0]);
	std::copy(snap.AmbientHeat.begin(), snap.AmbientHeat.end(), &hv[0][0]);
	std::copy(snap.Particles.begin(), snap.Particles.end(), parts);
	std::copy(snap.PortalParticles.begin(), snap.PortalParticles.end(), &portalp[0][0][0][0]);
	std::copy(snap.WirelessData.begin(), snap.WirelessData.end(), &wireless[0][0]);
	std::copy(snap.GravVelocityX.begin(), snap.GravVelocityX.end(), gravx);
	std::copy(snap.GravVelocityY.begin(), snap.GravVelocityY.end(), gravy);
	std::copy(snap.GravValue.begin(), snap.GravValue.end(), gravp);
	std::copy(snap.GravMap.begin(), snap.GravMap.end(), gravmap);
	std::copy(snap.BlockMap.begin(), snap.BlockMap.end(), &bmap[0][0]);
	std::copy(snap.ElecMap.begin(), snap.ElecMap.end(), &emap[0][0]);
	std::copy(snap.FanVelocityX.begin(), snap.FanVelocityX.end(), &fvx[0][0]);
	std::copy(snap.FanVelocityY.begin(), snap.FanVelocityY.end(), &fvy[0][0]);
	std::copy(snap.stickmen.begin(), snap.stickmen.end()-2, &fighters[0]);
	player = snap.stickmen[snap.stickmen.size()-1];
	player2 = snap.stickmen[snap.stickmen.size()-2];
	signs = snap.signs;
	CalculateBlockerWall();
}

void Simulation::clear_area(int area_x, int area_y, int area_w, int area_h)
{
	int cx = 0;
	int cy = 0;
	for (cy=0; cy<=area_h; cy++)
	{
		for (cx=0; cx<=area_w; cx++)
		{
			if(bmap[(cy+area_y)/CELL][(cx+area_x)/CELL] == WL_GRAV)
				gravWallChanged = true;
			bmap[(cy+area_y)/CELL][(cx+area_x)/CELL] = 0;
			emap[(cy+area_y)/CELL][(cx+area_x)/CELL] = 0;
			delete_part(cx+area_x, cy+area_y);
		}
	}
	for(int i = signs.size()-1; i >= 0; i--)
	{
		if(signs[i].text.length() && signs[i].x >= area_x && signs[i].y >= area_y && signs[i].x <= area_x+area_w && signs[i].y <= area_y+area_h)
		{
			signs.erase(signs.begin()+i);
		}
	}
}

bool Simulation::FloodFillPmapCheck(int x, int y, int type)
{
	if (type == 0)
		return !pmap[y][x] && !photons[y][x];
	if (elements[type].Properties&TYPE_ENERGY)
		return (photons[y][x]&0xFF) == type;
	else
		return (pmap[y][x]&0xFF) == type;
}

int Simulation::flood_prop_2(int x, int y, size_t propoffset, void * propvalue, StructProperty::PropertyType proptype, int parttype, char * bitmap)
{
	int x1, x2, i, dy = 1;
	x1 = x2 = x;
	while (x1>=CELL)
	{
		if (!FloodFillPmapCheck(x1-1, y, parttype) || bitmap[(y*XRES)+x1-1])
		{
			break;
		}
		x1--;
	}
	while (x2<XRES-CELL)
	{
		if (!FloodFillPmapCheck(x2+1, y, parttype) || bitmap[(y*XRES)+x2+1])
		{
			break;
		}
		x2++;
	}
	for (x=x1; x<=x2; x++)
	{
		i = pmap[y][x];
		if (!i)
			i = photons[y][x];
		if (!i)
			continue;
		switch (proptype) {
			case StructProperty::Float:
				*((float*)(((char*)&parts[i>>8])+propoffset)) = *((float*)propvalue);
				break;
				
			case StructProperty::ParticleType:
			case StructProperty::Integer:
				*((int*)(((char*)&parts[i>>8])+propoffset)) = *((int*)propvalue);
				break;
				
			case StructProperty::UInteger:
				*((unsigned int*)(((char*)&parts[i>>8])+propoffset)) = *((unsigned int*)propvalue);
				break;
				
			default:
				break;
		}
		bitmap[(y*XRES)+x] = 1;
	}
	if (y>=CELL+dy)
		for (x=x1; x<=x2; x++)
			if (FloodFillPmapCheck(x, y-dy, parttype) && !bitmap[((y-dy)*XRES)+x])
				if (!flood_prop_2(x, y-dy, propoffset, propvalue, proptype, parttype, bitmap))
					return 0;
	if (y<YRES-CELL-dy)
		for (x=x1; x<=x2; x++)
			if (FloodFillPmapCheck(x, y+dy, parttype) && !bitmap[((y+dy)*XRES)+x])
				if (!flood_prop_2(x, y+dy, propoffset, propvalue, proptype, parttype, bitmap))
					return 0;
	return 1;
}

int Simulation::flood_prop(int x, int y, size_t propoffset, void * propvalue, StructProperty::PropertyType proptype)
{
	int r = 0;
	char * bitmap = (char *)malloc(XRES*YRES); //Bitmap for checking
	memset(bitmap, 0, XRES*YRES);
	r = pmap[y][x];
	if (!r)
		r = photons[y][x];
	if (!r)
		return 1;
	flood_prop_2(x, y, propoffset, propvalue, proptype, r&0xFF, bitmap);
	free(bitmap);
	return 0;
}

SimulationSample Simulation::GetSample(int x, int y)
{
	SimulationSample sample;
	sample.PositionX = x;
	sample.PositionY = y;
	if (x >= 0 && x < XRES && y >= 0 && y < YRES)
	{
		if (photons[y][x])
		{
			sample.particle = parts[photons[y][x]>>8];
			sample.ParticleID = photons[y][x]>>8;
		}
		else if (pmap[y][x])
		{
			sample.particle = parts[pmap[y][x]>>8];
			sample.ParticleID = pmap[y][x]>>8;
		}
		if (bmap[y/CELL][x/CELL])
		{
			sample.WallType = bmap[y/CELL][x/CELL];
		}
		sample.AirPressure = pv[y/CELL][x/CELL];
		sample.AirTemperature = hv[y/CELL][x/CELL];
		sample.AirVelocityX = vx[y/CELL][x/CELL];
		sample.AirVelocityY = vy[y/CELL][x/CELL];

		if(grav->ngrav_enable)
		{
			sample.Gravity = gravp[(y/CELL)*(XRES/CELL)+(x/CELL)];
			sample.GravityVelocityX = gravx[(y/CELL)*(XRES/CELL)+(x/CELL)];
			sample.GravityVelocityY = gravy[(y/CELL)*(XRES/CELL)+(x/CELL)];
		}
	}
	else
		sample.isMouseInSim = false;

	sample.NumParts = NUM_PARTS;
	return sample;
}

#define PMAP_CMP_CONDUCTIVE(pmap, t) (((pmap)&0xFF)==(t) || (((pmap)&0xFF)==PT_SPRK && parts[(pmap)>>8].ctype==(t)))

int Simulation::FloodINST(int x, int y, int fullc, int cm)
{
	int c = fullc&0xFF;
	int x1, x2;
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int created_something = 0;

	if (c>=PT_NUM)
		return 0;

	if (cm==-1)
	{
		if (c==0)
		{
			cm = pmap[y][x]&0xFF;
			if (!cm)
				return 0;
		}
		else
			cm = 0;
	}

	if ((pmap[y][x]&0xFF)!=cm || parts[pmap[y][x]>>8].life!=0)
		return 1;

	coord_stack = (short unsigned int (*)[2])malloc(sizeof(unsigned short)*2*coord_stack_limit);
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;

	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		// go left as far as possible
		while (x1>=CELL)
		{
			if ((pmap[y][x1-1]&0xFF)!=cm || parts[pmap[y][x1-1]>>8].life!=0)
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (x2<XRES-CELL)
		{
			if ((pmap[y][x2+1]&0xFF)!=cm || parts[pmap[y][x2+1]>>8].life!=0)
			{
				break;
			}
			x2++;
		}
		// fill span
		for (x=x1; x<=x2; x++)
		{
			if (create_part(-1, x, y, fullc)>=0)
				created_something = 1;
		}

		// add vertically adjacent pixels to stack
		// (wire crossing for INST)
		if (y>=CELL+1 && x1==x2 &&
				PMAP_CMP_CONDUCTIVE(pmap[y-1][x1-1], cm) && PMAP_CMP_CONDUCTIVE(pmap[y-1][x1], cm) && PMAP_CMP_CONDUCTIVE(pmap[y-1][x1+1], cm) &&
				!PMAP_CMP_CONDUCTIVE(pmap[y-2][x1-1], cm) && PMAP_CMP_CONDUCTIVE(pmap[y-2][x1], cm) && !PMAP_CMP_CONDUCTIVE(pmap[y-2][x1+1], cm))
		{
			// travelling vertically up, skipping a horizontal line
			if ((pmap[y-2][x1]&0xFF)==cm && !parts[pmap[y-2][x1]>>8].life)
			{
				coord_stack[coord_stack_size][0] = x1;
				coord_stack[coord_stack_size][1] = y-2;
				coord_stack_size++;
				if (coord_stack_size>=coord_stack_limit)
				{
					free(coord_stack);
					return -1;
				}
			}
		}
		else if (y>=CELL+1)
		{
			for (x=x1; x<=x2; x++)
			{
				if ((pmap[y-1][x]&0xFF)==cm && !parts[pmap[y-1][x]>>8].life)
				{
					if (x==x1 || x==x2 || y>=YRES-CELL-1 || !PMAP_CMP_CONDUCTIVE(pmap[y+1][x], cm) || PMAP_CMP_CONDUCTIVE(pmap[y+1][x+1], cm) || PMAP_CMP_CONDUCTIVE(pmap[y+1][x-1], cm))
					{
						// if at the end of a horizontal section, or if it's a T junction or not a 1px wire crossing
						coord_stack[coord_stack_size][0] = x;
						coord_stack[coord_stack_size][1] = y-1;
						coord_stack_size++;
						if (coord_stack_size>=coord_stack_limit)
						{
							free(coord_stack);
							return -1;
						}
					}
				}
			}
		}

		if (y<YRES-CELL-1 && x1==x2 &&
				PMAP_CMP_CONDUCTIVE(pmap[y+1][x1-1], cm) && PMAP_CMP_CONDUCTIVE(pmap[y+1][x1], cm) && PMAP_CMP_CONDUCTIVE(pmap[y+1][x1+1], cm) &&
				!PMAP_CMP_CONDUCTIVE(pmap[y+2][x1-1], cm) && PMAP_CMP_CONDUCTIVE(pmap[y+2][x1], cm) && !PMAP_CMP_CONDUCTIVE(pmap[y+2][x1+1], cm))
		{
			// travelling vertically down, skipping a horizontal line
			if ((pmap[y+2][x1]&0xFF)==cm && !parts[pmap[y+2][x1]>>8].life)
			{
				coord_stack[coord_stack_size][0] = x1;
				coord_stack[coord_stack_size][1] = y+2;
				coord_stack_size++;
				if (coord_stack_size>=coord_stack_limit)
				{
					free(coord_stack);
					return -1;
				}
			}
		}
		else if (y<YRES-CELL-1)
		{
			for (x=x1; x<=x2; x++)
			{
				if ((pmap[y+1][x]&0xFF)==cm && !parts[pmap[y+1][x]>>8].life)
				{
					if (x==x1 || x==x2 || y<0 || !PMAP_CMP_CONDUCTIVE(pmap[y-1][x], cm) || PMAP_CMP_CONDUCTIVE(pmap[y-1][x+1], cm) || PMAP_CMP_CONDUCTIVE(pmap[y-1][x-1], cm))
					{
						// if at the end of a horizontal section, or if it's a T junction or not a 1px wire crossing
						coord_stack[coord_stack_size][0] = x;
						coord_stack[coord_stack_size][1] = y+1;
						coord_stack_size++;
						if (coord_stack_size>=coord_stack_limit)
						{
							free(coord_stack);
							return -1;
						}
					}

				}
			}
		}
	} while (coord_stack_size>0);
	free(coord_stack);
	return created_something;
}

int Simulation::flood_water(int x, int y, int i, int originaly, int check)
{
	int x1 = 0,x2 = 0;
	// go left as far as possible
	x1 = x2 = x;
	if (!pmap[y][x])
		return 1;

	while (x1>=CELL)
	{
		if ((elements[(pmap[y][x1-1]&0xFF)].Falldown)!=2)
		{
			break;
		}
		x1--;
	}
	while (x2<XRES-CELL)
	{
		if ((elements[(pmap[y][x2+1]&0xFF)].Falldown)!=2)
		{
			break;
		}
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
	{
		if (check)
			parts[pmap[y][x]>>8].flags &= ~FLAG_WATEREQUAL;//flag it as checked (different from the original particle's checked flag)
		else
			parts[pmap[y][x]>>8].flags |= FLAG_WATEREQUAL;
		//check above, maybe around other sides too?
		if ( ((y-1) > originaly) && !pmap[y-1][x] && eval_move(parts[i].type, x, y-1, NULL))
		{
			int oldx = (int)(parts[i].x + 0.5f);
			int oldy = (int)(parts[i].y + 0.5f);
			pmap[y-1][x] = pmap[oldy][oldx];
			pmap[oldy][oldx] = 0;
			parts[i].x = x;
			parts[i].y = y-1;
			return 0;
		}
	}
	// fill children

	if (y>=CELL+1)
		for (x=x1; x<=x2; x++)
			if ((elements[(pmap[y-1][x]&0xFF)].Falldown)==2 && (parts[pmap[y-1][x]>>8].flags & FLAG_WATEREQUAL) == check)
				if (!flood_water(x, y-1, i, originaly, check))
					return 0;
	if (y<YRES-CELL-1)
		for (x=x1; x<=x2; x++)
			if ((elements[(pmap[y+1][x]&0xFF)].Falldown)==2 && (parts[pmap[y+1][x]>>8].flags & FLAG_WATEREQUAL) == check)
				if (!flood_water(x, y+1, i, originaly, check))
					return 0;
	return 1;
}

void Simulation::SetEdgeMode(int newEdgeMode)
{
	edgeMode = newEdgeMode;
	switch(edgeMode)
	{
	case 0:
		for(int i = 0; i<(XRES/CELL); i++)
		{
			bmap[0][i] = 0;
			bmap[YRES/CELL-1][i] = 0;
		}
		for(int i = 1; i<((YRES/CELL)-1); i++)
		{
			bmap[i][0] = 0;
			bmap[i][XRES/CELL-1] = 0;
		}
		break;
	case 1:
		int i;
		for(i=0; i<(XRES/CELL); i++)
		{
			bmap[0][i] = WL_WALL;
			bmap[YRES/CELL-1][i] = WL_WALL;
		}
		for(i=1; i<((YRES/CELL)-1); i++)
		{
			bmap[i][0] = WL_WALL;
			bmap[i][XRES/CELL-1] = WL_WALL;
		}
		break;
	default:
		SetEdgeMode(0);
	}
}

void Simulation::ApplyDecoration(int x, int y, int colR_, int colG_, int colB_, int colA_, int mode)
{
	int rp;
	float tr, tg, tb, ta, colR = colR_, colG = colG_, colB = colB_, colA = colA_;
	float strength = 0.01f;
	rp = pmap[y][x];
	if (!rp)
		rp = photons[y][x];
	if (!rp)
		return;

	ta = (parts[rp>>8].dcolour>>24)&0xFF;
	tr = (parts[rp>>8].dcolour>>16)&0xFF;
	tg = (parts[rp>>8].dcolour>>8)&0xFF;
	tb = (parts[rp>>8].dcolour)&0xFF;

	ta /= 255.0f; tr /= 255.0f; tg /= 255.0f; tb /= 255.0f;
	colR /= 255.0f; colG /= 255.0f; colB /= 255.0f; colA /= 255.0f;

	if (mode == DECO_DRAW)
	{
		ta = colA;
		tr = colR;
		tg = colG;
		tb = colB;
	}
	else if (mode == DECO_CLEAR)
	{
		ta = tr = tg = tb = 0.0f;
	}
	else if (mode == DECO_ADD)
	{
		//ta += (colA*strength)*colA;
		tr += (colR*strength)*colA;
		tg += (colG*strength)*colA;
		tb += (colB*strength)*colA;
	}
	else if (mode == DECO_SUBTRACT)
	{
		//ta -= (colA*strength)*colA;
		tr -= (colR*strength)*colA;
		tg -= (colG*strength)*colA;
		tb -= (colB*strength)*colA;
	}
	else if (mode == DECO_MULTIPLY)
	{
		tr *= 1.0f+(colR*strength)*colA;
		tg *= 1.0f+(colG*strength)*colA;
		tb *= 1.0f+(colB*strength)*colA;
	}
	else if (mode == DECO_DIVIDE)
	{
		tr /= 1.0f+(colR*strength)*colA;
		tg /= 1.0f+(colG*strength)*colA;
		tb /= 1.0f+(colB*strength)*colA;
	}
	else if (mode == DECO_SMUDGE)
	{
		float tas = 0.0f, trs = 0.0f, tgs = 0.0f, tbs = 0.0f;
		
		int rx, ry;
		float num = 0;	
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
			{
				if (abs(rx)+abs(ry) > 2 && (pmap[y+ry][x+rx]&0xFF) && parts[pmap[y+ry][x+rx]>>8].dcolour)
				{
					Particle part = parts[pmap[y+ry][x+rx]>>8];
					num += 1.0f;
					tas += ((float)((part.dcolour>>24)&0xFF));
					trs += ((float)((part.dcolour>>16)&0xFF));
					tgs += ((float)((part.dcolour>>8)&0xFF));
					tbs += ((float)((part.dcolour)&0xFF));
				}
			}
		if (num == 0)
			return;
		ta = (tas/num)/255.0f;
		tr = (trs/num)/255.0f;
		tg = (tgs/num)/255.0f;
		tb = (tbs/num)/255.0f;
		if (!parts[rp>>8].dcolour)
			ta -= 3/255.0f;
	}

	ta *= 255.0f; tr *= 255.0f; tg *= 255.0f; tb *= 255.0f;
	ta += .5f; tr += .5f; tg += .5f; tb += .5f;

	colA_ = ta;
	colR_ = tr;
	colG_ = tg;
	colB_ = tb;

	if(colA_ > 255)
		colA_ = 255;
	else if(colA_ < 0)
		colA_ = 0;
	if(colR_ > 255)
		colR_ = 255;
	else if(colR_ < 0)
		colR_ = 0;
	if(colG_ > 255)
		colG_ = 255;
	else if(colG_ < 0)
		colG_ = 0;
	if(colB_ > 255)
		colB_ = 255;
	else if(colB_ < 0)
		colB_ = 0;
	parts[rp>>8].dcolour = ((colA_<<24)|(colR_<<16)|(colG_<<8)|colB_);
}

void Simulation::ApplyDecorationPoint(int positionX, int positionY, int colR, int colG, int colB, int colA, int mode, Brush * cBrush)
{
	if(cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();

		for(int y = 0; y < sizeY; y++)
		{
			for(int x = 0; x < sizeX; x++)
			{
				if(bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
				{
					ApplyDecoration(positionX+(x-radiusX), positionY+(y-radiusY), colR, colG, colB, colA, mode);
				}
			}
		}
	}
}

void Simulation::ApplyDecorationLine(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode, Brush * cBrush)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx, ry;
	float e = 0.0f, de;

	if(cBrush)
	{
		rx = cBrush->GetRadius().X;
		ry = cBrush->GetRadius().Y;
	}

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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			ApplyDecorationPoint(y, x, colR, colG, colB, colA, mode, cBrush);
		else
			ApplyDecorationPoint(x, y, colR, colG, colB, colA, mode, cBrush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry))
			{
				if (reverseXY)
					ApplyDecorationPoint(y, x, colR, colG, colB, colA, mode, cBrush);
				else
					ApplyDecorationPoint(x, y, colR, colG, colB, colA, mode, cBrush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::ApplyDecorationBox(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode)
{
	int i, j;

	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			ApplyDecoration(i, j, colR, colG, colB, colA, mode);
}

int Simulation::Tool(int x, int y, int tool, float strength)
{
	if(tools[tool])
	{
		Particle * cpart = NULL;
		int r;
		if(r = pmap[y][x])
			cpart = &(parts[r>>8]);
		else if(r = photons[y][x])
			cpart = &(parts[r>>8]);
		return tools[tool]->Perform(this, cpart, x, y, strength);
	}
	return 0;
}

int Simulation::ToolBrush(int positionX, int positionY, int tool, Brush * cBrush, float strength)
{
	if(cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();
		for(int y = 0; y < sizeY; y++)
			for(int x = 0; x < sizeX; x++)
				if(bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
					Tool(positionX+(x-radiusX), positionY+(y-radiusY), tool, strength);
	}
	return 0;
}

void Simulation::ToolLine(int x1, int y1, int x2, int y2, int tool, Brush * cBrush, float strength)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = cBrush->GetRadius().X, ry = cBrush->GetRadius().Y;
	float e = 0.0f, de;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			ToolBrush(y, x, tool, cBrush, strength);
		else
			ToolBrush(x, y, tool, cBrush, strength);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					ToolBrush(y, x, tool, cBrush, strength);
				else
					ToolBrush(x, y, tool, cBrush, strength);
			}
			e -= 1.0f;
		}
	}
}
void Simulation::ToolBox(int x1, int y1, int x2, int y2, int tool, float strength)
{
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			Tool(i, j, tool, strength);
}

void Simulation::CalculateBlockerWall() 
{
	int wallX, wallY, id = 0, list_start = 1, list_end = 1, px, py, rx, ry;
	bool (*hasGone)[XRES/CELL];
	unsigned short (*list)[2];
	hasGone = (bool (*)[XRES/CELL])malloc(sizeof(bool) * (XRES/CELL) * (YRES/CELL));
	list = (short unsigned int (*)[2])malloc(sizeof(unsigned short)*2*(XRES/CELL)*(YRES/CELL));
	for (wallX = 0;wallX < XRES / CELL;wallX++) {
		for (wallY = 0;wallY < YRES / CELL;wallY++) {
			if(bmap[wallY][wallX] != WL_BLOCKER) {
				BlockerWall[wallY][wallX] = 0;
			} else if (!hasGone[wallY][wallX]) {
				id++;
				if (id >= BLOCKER_CHANNELS) return;
				hasGone[wallY][wallX] = true;
				list[list_start][1] = wallX;
				list[list_start][2] = wallY;
				do {
					px = list[list_start][1];
					py = list[list_start][2];
					BlockerWall[py][px] = id;
					list_start++;
					for (rx = -1; rx <= 1;rx++) {
						for (ry = -1; ry <= 1;ry++) {
							if (bmap[py+ry][px+rx] == WL_BLOCKER && !hasGone[py+ry][px+rx]) {
								list_end++;
								list[list_end][1] = px+rx;
								list[list_end][2] = py+ry;
								hasGone[py+ry][px+rx] = true;
							}
						}
					}
				} while (list_start <= list_end);
			}
		}
	}
	free(hasGone);
	free(list);
}

int Simulation::CreateWalls(int x, int y, int rx, int ry, int wall, Brush * cBrush)
{
	if(cBrush)
	{
		rx = cBrush->GetRadius().X;
		ry = cBrush->GetRadius().Y;
	}
	
	ry = ry/CELL;
	rx = rx/CELL;
	x = x/CELL;
	y = y/CELL;
	x -= rx;
	y -= ry;
	for (int wallX = x; wallX <= x+rx+rx; wallX++)
	{
		for (int wallY = y; wallY <= y+ry+ry; wallY++)
		{
			if (wallX >= 0 && wallX < XRES/CELL && wallY >= 0 && wallY < YRES/CELL)
			{
				if (wall == WL_FAN)
				{
					fvx[wallY][wallX] = 0.0f;
					fvy[wallY][wallX] = 0.0f;
				}
				else if (wall == WL_STREAM)
				{
					wallX = x + rx;
					wallY = y + ry;
					//streamlines can't be drawn next to each other
					for (int tempY = wallY-1; tempY < wallY+2; tempY++)
						for (int tempX = wallX-1; tempX < wallX+2; tempX++)
						{
							if (tempX >= 0 && tempX < XRES/CELL && tempY >= 0 && tempY < YRES/CELL && bmap[tempY][tempX] == WL_STREAM)
								return 1;
						}
				}
				if (wall == WL_GRAV || bmap[wallY][wallX] == WL_GRAV)
					gravWallChanged = true;
				bmap[wallY][wallX] = wall;
			}
		}
	}
	if ((wall == WL_BLOCKER || wall == WL_ERASE) && !calculated) {
		CalculateBlockerWall();
	}
	return 1;
}

void Simulation::CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int wall, Brush * cBrush)
{
	int x, y, dx, dy, sy;
	calculated = true;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	float e = 0.0f, de;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			CreateWalls(y, x, rx, ry, wall, cBrush);
		else
			CreateWalls(x, y, rx, ry, wall, cBrush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateWalls(y, x, rx, ry, wall, cBrush);
				else
					CreateWalls(x, y, rx, ry, wall, cBrush);
			}
			e -= 1.0f;
		}
	}
	if (wall == WL_BLOCKER || wall == WL_ERASE) {
		CalculateBlockerWall();
	}
	calculated = false;
}

void Simulation::CreateWallBox(int x1, int y1, int x2, int y2, int wall)
{
	int i, j;
	calculated = true;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			CreateWalls(i, j, 0, 0, wall, NULL);
	if (wall == WL_BLOCKER || wall == WL_ERASE) {
		CalculateBlockerWall();
	}
	calculated = false;
}

int Simulation::FloodWalls(int x, int y, int wall, int bm)
{
	int x1, x2, dy = CELL;
	if (bm==-1)
	{
		if (wall==WL_ERASE)
		{
			bm = bmap[y/CELL][x/CELL];
			if (!bm)
				return 0;
		}
		else
			bm = 0;
	}
	
	if (bmap[y/CELL][x/CELL]!=bm)
		return 1;
	
	// go left as far as possible
	x1 = x2 = x;
	while (x1>=CELL)
	{
		if (bmap[y/CELL][(x1-1)/CELL]!=bm)
		{
			break;
		}
		x1--;
	}
	while (x2<XRES-CELL)
	{
		if (bmap[y/CELL][(x2+1)/CELL]!=bm)
		{
			break;
		}
		x2++;
	}
	
	// fill span
	for (x=x1; x<=x2; x++)
	{
		if (!CreateWalls(x, y, 0, 0, wall, NULL))
			return 0;
	}
	// fill children
	if (y>=CELL)
		for (x=x1; x<=x2; x++)
			if (bmap[(y-dy)/CELL][x/CELL]==bm)
				if (!FloodWalls(x, y-dy, wall, bm))
					return 0;
	if (y<YRES-CELL)
		for (x=x1; x<=x2; x++)
			if (bmap[(y+dy)/CELL][x/CELL]==bm)
				if (!FloodWalls(x, y+dy, wall, bm))
					return 0;
	return 1;
}

int Simulation::CreateParts(int positionX, int positionY, int c, Brush * cBrush, int flags)
{
	if (flags == -1)
		flags = replaceModeFlags;
	if(cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();
		
		for(int y = 0; y < sizeY; y++)
		{
			for(int x = 0; x < sizeX; x++)
			{
				if(bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
				{
					CreatePartFlags(positionX+(x-radiusX), positionY+(y-radiusY), c, flags);
				}
			}
		}
	}
	return 0;
}

int Simulation::CreateParts(int x, int y, int rx, int ry, int c, int flags)
{
	int i, j, f = 0;

	if (flags == -1)
		flags = replaceModeFlags;

	for (j=-ry; j<=ry; j++)
		for (i=-rx; i<=rx; i++)
			if (CreatePartFlags(x+i, y+j, c, flags))
				f = 1;
	return !f;
}

int Simulation::CreatePartFlags(int x, int y, int c, int flags)
{
	//delete
	if (c == 0 && !(flags&REPLACE_MODE))
		delete_part(x, y);
	//specific delete
	else if ((flags&SPECIFIC_DELETE) && !(flags&REPLACE_MODE))
	{
		if (!replaceModeSelected || (pmap[y][x]&0xFF) == replaceModeSelected || (photons[y][x]&0xFF) == replaceModeSelected)
			delete_part(x, y);
	}
	//replace mode
	else if (flags&REPLACE_MODE)
	{
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return 0;
		if (replaceModeSelected && (pmap[y][x]&0xFF) != replaceModeSelected && (photons[y][x]&0xFF) != replaceModeSelected)
			return 0;
		if ((pmap[y][x]))
		{
			delete_part(x, y);
			if (c!=0)
				create_part(-2, x, y, c);
		}
	}
	//normal draw
	else
		if (create_part(-2, x, y, c) == -1)
			return 1;
	return 0;
}

void Simulation::CreateLine(int x1, int y1, int x2, int y2, int c, Brush * cBrush, int flags)
{
	int x, y, dx, dy, sy, rx = cBrush->GetRadius().X, ry = cBrush->GetRadius().Y;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	float e = 0.0f, de;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			CreateParts(y, x, c, cBrush, flags);
		else
			CreateParts(x, y, c, cBrush, flags);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateParts(y, x, c, cBrush, flags);
				else
					CreateParts(x, y, c, cBrush, flags);
			}
			e -= 1.0f;
		}
	}
}

//Now simply creates a 0 pixel radius line without all the complicated flags / other checks
void Simulation::CreateLine(int x1, int y1, int x2, int y2, int c)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy;
	float e, de;
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			create_part(-2, y, x, c);
		else
			create_part(-2, x, y, c);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if ((y1<y2) ? (y<=y2) : (y>=y2))
			{
				if (reverseXY)
					create_part(-2, y, x, c);
				else
					create_part(-2, x, y, c);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateBox(int x1, int y1, int x2, int y2, int c, int flags)
{
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			CreateParts(i, j, 0, 0, c, flags);
}

int Simulation::FloodParts(int x, int y, int fullc, int cm, int flags)
{
	int c = fullc&0xFF;
	int x1, x2, dy = (c<PT_NUM)?1:CELL;
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int created_something = 0;
	
	if (cm==-1)
	{
		//if initial flood point is out of bounds, do nothing
		if (c != 0 && (x < CELL || x >= XRES-CELL || y < CELL || y >= YRES-CELL))
			return 1;
		else if (x < 0 || x >= XRES || y < 0 || y >= YRES)
			return 1;
		if (c==0)
		{
			cm = pmap[y][x]&0xFF;
			if (!cm)
				cm = photons[y][x]&0xFF;
			if (!cm)
			{
				if (bmap[y/CELL][x/CELL])
					return FloodWalls(x, y, WL_ERASE, -1);
				else
					return -1;
			}
		}
		else
			cm = 0;
	}
	if (c != 0 && IsWallBlocking(x, y, c))
		return 1;

	if (!FloodFillPmapCheck(x, y, cm))
		return 1;

	coord_stack = (short unsigned int (*)[2])malloc(sizeof(unsigned short)*2*coord_stack_limit);
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;

	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		// go left as far as possible
		while (c?x1>CELL:x1>0)
		{
			if (!FloodFillPmapCheck(x1-1, y, cm) || (c != 0 && IsWallBlocking(x1-1, y, c)))
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (c?x2<XRES-CELL-1:x2<XRES-1)
		{
			if (!FloodFillPmapCheck(x2+1, y, cm) || (c != 0 && IsWallBlocking(x2+1, y, c)))
			{
				break;
			}
			x2++;
		}
		// fill span
		for (x=x1; x<=x2; x++)
		{
			if (CreateParts(x, y, 0, 0, fullc, flags))
				created_something = 1;
		}

		if (c?y>=CELL+dy:y>=dy)
			for (x=x1; x<=x2; x++)
				if (FloodFillPmapCheck(x, y-dy, cm) && (c == 0 || !IsWallBlocking(x, y-dy, c)))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y-dy;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						free(coord_stack);
						return -1;
					}
				}

		if (c?y<YRES-CELL-dy:y<YRES-dy)
			for (x=x1; x<=x2; x++)
				if (FloodFillPmapCheck(x, y+dy, cm) && (c == 0 || !IsWallBlocking(x, y+dy, c)))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y+dy;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						free(coord_stack);
						return -1;
					}
				}
	} while (coord_stack_size>0);
	free(coord_stack);
	return created_something;
}

void *Simulation::transform_save(void *odata, int *size, matrix2d transform, vector2d translate)
{
	void *ndata;
	unsigned char (*bmapo)[XRES/CELL] = (unsigned char (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(unsigned char));
	unsigned char (*bmapn)[XRES/CELL] = (unsigned char (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(unsigned char));
	Particle *partst = (Particle*)calloc(sizeof(Particle), NPART);
	sign *signst = (sign*)calloc(MAXSIGNS, sizeof(sign));
	unsigned (*pmapt)[XRES] = (unsigned (*)[XRES])calloc(YRES*XRES, sizeof(unsigned));
	float (*fvxo)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvyo)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvxn)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvyn)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vxo)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vyo)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vxn)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vyn)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*pvo)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*pvn)[XRES/CELL] = (float (*)[XRES/CELL])calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	int i, x, y, nx, ny, w, h, nw, nh;
	vector2d pos, tmp, ctl, cbr;
	vector2d vel;
	vector2d cornerso[4];
	unsigned char *odatac = (unsigned char *)odata;
	//if (parse_save(odata, *size, 0, 0, 0, bmapo, vxo, vyo, pvo, fvxo, fvyo, signst, partst, pmapt)) //TODO: Implement
	{
		free(bmapo);
		free(bmapn);
		free(partst);
		free(signst);
		free(pmapt);
		free(fvxo);
		free(fvyo);
		free(fvxn);
		free(fvyn);
		free(vxo);
		free(vyo);
		free(vxn);
		free(vyn);
		free(pvo);
		free(pvn);
		return odata;
	}
	w = odatac[6]*CELL;
	h = odatac[7]*CELL;
	// undo any translation caused by rotation
	cornerso[0] = v2d_new(0,0);
	cornerso[1] = v2d_new(w-1,0);
	cornerso[2] = v2d_new(0,h-1);
	cornerso[3] = v2d_new(w-1,h-1);
	for (i=0; i<4; i++)
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
	nw = floor(cbr.x+0.5f)-floor(ctl.x+0.5f)+1;
	nh = floor(cbr.y+0.5f)-floor(ctl.y+0.5f)+1;
	if (nw>XRES) nw = XRES;
	if (nh>YRES) nh = YRES;
	// rotate and translate signs, parts, walls
	for (i=0; i<MAXSIGNS; i++)
	{
		if (!signst[i].text[0]) continue;
		pos = v2d_new(signst[i].x, signst[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx<0 || nx>=nw || ny<0 || ny>=nh)
		{
			signst[i].text[0] = 0;
			continue;
		}
		signst[i].x = nx;
		signst[i].y = ny;
	}
	for (i=0; i<NPART; i++)
	{
		if (!partst[i].type) continue;
		pos = v2d_new(partst[i].x, partst[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx<0 || nx>=nw || ny<0 || ny>=nh)
		{
			partst[i].type = PT_NONE;
			continue;
		}
		partst[i].x = nx;
		partst[i].y = ny;
		vel = v2d_new(partst[i].vx, partst[i].vy);
		vel = m2d_multiply_v2d(transform, vel);
		partst[i].vx = vel.x;
		partst[i].vy = vel.y;
	}
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			pos = v2d_new(x*CELL+CELL*0.4f, y*CELL+CELL*0.4f);
			pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
			nx = pos.x/CELL;
			ny = pos.y/CELL;
			if (nx<0 || nx>=nw/CELL || ny<0 || ny>=nh/CELL)
				continue;
			if (bmapo[y][x])
			{
				bmapn[ny][nx] = bmapo[y][x];
				if (bmapo[y][x]==WL_FAN)
				{
					vel = v2d_new(fvxo[y][x], fvyo[y][x]);
					vel = m2d_multiply_v2d(transform, vel);
					fvxn[ny][nx] = vel.x;
					fvyn[ny][nx] = vel.y;
				}
			}
			vel = v2d_new(vxo[y][x], vyo[y][x]);
			vel = m2d_multiply_v2d(transform, vel);
			vxn[ny][nx] = vel.x;
			vyn[ny][nx] = vel.y;
			pvn[ny][nx] = pvo[y][x];
		}
	//ndata = build_save(size,0,0,nw,nh,bmapn,vxn,vyn,pvn,fvxn,fvyn,signst,partst); //TODO: IMPLEMENT
	free(bmapo);
	free(bmapn);
	free(partst);
	free(signst);
	free(pmapt);
	free(fvxo);
	free(fvyo);
	free(fvxn);
	free(fvyn);
	free(vxo);
	free(vyo);
	free(vxn);
	free(vyn);
	free(pvo);
	free(pvn);
	return ndata;
}

TPT_NO_INLINE void Simulation::orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[])
{
	resblock1[0] = (block1&0x000000FF);
	resblock1[1] = (block1&0x0000FF00)>>8;
	resblock1[2] = (block1&0x00FF0000)>>16;
	resblock1[3] = (block1&0xFF000000)>>24;

	resblock2[0] = (block2&0x000000FF);
	resblock2[1] = (block2&0x0000FF00)>>8;
	resblock2[2] = (block2&0x00FF0000)>>16;
	resblock2[3] = (block2&0xFF000000)>>24;
}

TPT_NO_INLINE void Simulation::orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[])
{
	int block1tmp = 0;
	int block2tmp = 0;

	block1tmp = (resblock1[0]&0xFF);
	block1tmp |= (resblock1[1]&0xFF)<<8;
	block1tmp |= (resblock1[2]&0xFF)<<16;
	block1tmp |= (resblock1[3]&0xFF)<<24;

	block2tmp = (resblock2[0]&0xFF);
	block2tmp |= (resblock2[1]&0xFF)<<8;
	block2tmp |= (resblock2[2]&0xFF)<<16;
	block2tmp |= (resblock2[3]&0xFF)<<24;

	*block1 = block1tmp;
	*block2 = block2tmp;
}

inline int Simulation::is_wire(int x, int y)
{
	return bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE;
}

inline int Simulation::is_wire_off(int x, int y)
{
	return (bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE) && emap[y][x]<8;
}

int Simulation::get_wavelength_bin(int *wm)
{
	int i, w0=30, wM=0;

	if (!*wm)
		return -1;

	for (i=0; i<30; i++)
		if (*wm & (1<<i)) {
			if (i < w0)
				w0 = i;
			if (i > wM)
				wM = i;
		}

	if (wM-w0 < 5)
		return (wM+w0)/2;

	i = rand() % (wM-w0-3);
	i += w0;

	*wm &= 0x1F << i;
	return i + 2;
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
	while (x2<XRES/CELL-1)
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
				if (x==x1 || x==x2 || y>=YRES/CELL-1 ||
				        is_wire(x-1, y-1) || is_wire(x+1, y-1) ||
				        is_wire(x-1, y+1) || !is_wire(x, y+1) || is_wire(x+1, y+1))
					set_emap(x, y-1);
			}

	if (y<YRES/CELL-2 && x1==x2 &&
	        is_wire(x1-1, y+1) && is_wire(x1, y+1) && is_wire(x1+1, y+1) &&
	        !is_wire(x1-1, y+2) && is_wire(x1, y+2) && !is_wire(x1+1, y+2))
		set_emap(x1, y+2);
	else if (y<YRES/CELL-1)
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
			return parts[pmr>>8].type;
		else
			return PT_NONE;
	}
	else
	{
		int pmr2 = pmap[(int)((parts[ci].y + parts[ni].y)/2+0.5f)][(int)((parts[ci].x + parts[ni].x)/2+0.5f)];//seems to be more accurate.
		if (pmr2)
		{
			if (parts[pmr2>>8].type==t)
				return t;
		}
		else
			return PT_NONE;
	}
	return PT_NONE;
}


int Simulation::nearest_part(int ci, int t, int max_d)
{
	int distance = (max_d!=-1)?max_d:MAX_DISTANCE;
	int ndistance = 0;
	int id = -1;
	int i = 0;
	int cx = (int)parts[ci].x;
	int cy = (int)parts[ci].y;
	for (i=0; i<=parts_lastActiveIndex; i++)
	{
		if ((parts[i].type==t||(t==-1&&parts[i].type))&&!parts[i].life&&i!=ci)
		{
			ndistance = abs(cx-parts[i].x)+abs(cy-parts[i].y);// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
			if (ndistance<distance)
			{
				distance = ndistance;
				id = i;
			}
		}
	}
	return id;
}

void Simulation::create_arc(int sx, int sy, int dx, int dy, int midpoints, int variance, int type, int flags)
{
	int i;
	float xint, yint;
	int *xmid, *ymid;
	int voffset = variance/2;
	xmid = (int *)calloc(midpoints + 2, sizeof(int));
	ymid = (int *)calloc(midpoints + 2, sizeof(int));
	xint = (float)(dx-sx)/(float)(midpoints+1.0f);
	yint = (float)(dy-sy)/(float)(midpoints+1.0f);
	xmid[0] = sx;
	xmid[midpoints+1] = dx;
	ymid[0] = sy;
	ymid[midpoints+1] = dy;

	for(i = 1; i <= midpoints; i++)
	{
		ymid[i] = ymid[i-1]+yint;
		xmid[i] = xmid[i-1]+xint;
	}

	for(i = 0; i <= midpoints; i++)
	{
		if(i!=midpoints)
		{
			xmid[i+1] += (rand()%variance)-voffset;
			ymid[i+1] += (rand()%variance)-voffset;
		}
		CreateLine(xmid[i], ymid[i], xmid[i+1], ymid[i+1], type);
	}
	free(xmid);
	free(ymid);
}

void Simulation::clear_sim(void)
{
	int i, x, y;
	emp_decor = 0;
	signs.clear();
	memset(bmap, 0, sizeof(bmap));
	memset(emap, 0, sizeof(emap));
	memset(parts, 0, sizeof(Particle)*NPART);
	for (i=0; i<NPART-1; i++)
		parts[i].life = i+1;
	parts[NPART-1].life = -1;
	pfree = 0;
	parts_lastActiveIndex = 0;
	memset(pmap, 0, sizeof(pmap));
	if(fvx)
		memset(fvx, 0, sizeof(fvx));
	if(fvy)
		memset(fvy, 0, sizeof(fvy));
	memset(photons, 0, sizeof(photons));
	memset(wireless, 0, sizeof(wireless));
	memset(gol2, 0, sizeof(gol2));
	memset(portalp, 0, sizeof(portalp));
	memset(fighters, 0, sizeof(fighters));
	std::fill(elementCount, elementCount+PT_NUM, 0);
	elementRecount = true;
	fighcount = 0;
	player.spwn = 0;
	player2.spwn = 0;
	//memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
	//memset(fire_r, 0, sizeof(fire_r));
	//memset(fire_g, 0, sizeof(fire_g));
	//memset(fire_b, 0, sizeof(fire_b));
	//if(gravmask)
		//memset(gravmask, 0xFFFFFFFF, (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
	if(grav)
		grav->Clear();
	if(air)
	{
		air->Clear();
		air->ClearAirH();
	}
	SetEdgeMode(edgeMode);
}

bool Simulation::IsWallBlocking(int x, int y, int type)
{
	if (bmap[y/CELL][x/CELL])
	{
		int wall = bmap[y/CELL][x/CELL];
		if (wall == WL_ALLOWGAS && !(elements[type].Properties&TYPE_GAS))
			return true;
		else if (wall == WL_ALLOWENERGY && !(elements[type].Properties&TYPE_ENERGY))
			return true;
		else if (wall == WL_ALLOWLIQUID && elements[type].Falldown!=2)
			return true;
		else if (wall == WL_ALLOWSOLID && elements[type].Falldown!=1)
			return true;
		else if (wall == WL_ALLOWAIR || wall == WL_WALL || wall == WL_WALLELEC)
			return true;
		else if (wall == WL_EWALL && !emap[y/CELL][x/CELL])
			return true;
	}
	return false;
}

void Simulation::init_can_move()
{
	int movingType, destinationType;
	// can_move[moving type][type at destination]
	//  0 = No move/Bounce
	//  1 = Swap
	//  2 = Both particles occupy the same space.
	//  3 = Varies, go run some extra checks
	
	//particles that don't exist shouldn't move...
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
		can_move[0][destinationType] = 0;

	//initialize everything else to swapping by default
	for (movingType = 1; movingType < PT_NUM; movingType++)
		for (destinationType = 0; destinationType < PT_NUM; destinationType++)
			can_move[movingType][destinationType] = 1;

	//photons go through everything by default
	for (destinationType = 1; destinationType < PT_NUM; destinationType++)
		can_move[PT_PHOT][destinationType] = 2;

	for (movingType = 1; movingType < PT_NUM; movingType++)
	{
		for (destinationType = 1; destinationType < PT_NUM; destinationType++)
		{
			//weight check, also prevents particles of same type displacing each other
			if (elements[movingType].Weight <= elements[destinationType].Weight || destinationType == PT_GEL)
				can_move[movingType][destinationType] = 0;

			//other checks for NEUT and energy particles
			if (movingType == PT_NEUT && (elements[destinationType].Properties&PROP_NEUTPASS))
				can_move[movingType][destinationType] = 2;
			if (movingType == PT_NEUT && (elements[destinationType].Properties&PROP_NEUTABSORB))
				can_move[movingType][destinationType] = 1;
			if (movingType == PT_NEUT && (elements[destinationType].Properties&PROP_NEUTPENETRATE))
				can_move[movingType][destinationType] = 1;
			if (destinationType == PT_NEUT && (elements[movingType].Properties&PROP_NEUTPENETRATE))
				can_move[movingType][destinationType] = 0;
			if ((elements[movingType].Properties&TYPE_ENERGY) && (elements[destinationType].Properties&TYPE_ENERGY))
				can_move[movingType][destinationType] = 2;
		}
	}
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		//set what stickmen can move through
		int stkm_move = 0;
		if (elements[destinationType].Properties & (TYPE_LIQUID | TYPE_GAS))
			stkm_move = 2;
		if (!destinationType || destinationType == PT_PRTO || destinationType == PT_SPAWN || destinationType == PT_SPAWN2)
			stkm_move = 2;
		can_move[PT_STKM][destinationType] = stkm_move;
		can_move[PT_STKM2][destinationType] = stkm_move;
		can_move[PT_FIGH][destinationType] = stkm_move;

		//spark shouldn't move
		can_move[PT_SPRK][destinationType] = 0;
	}
	for (movingType = 1; movingType < PT_NUM; movingType++)
	{
		//everything "swaps" with VACU and BHOL to make them eat things
		can_move[movingType][PT_BHOL] = 1;
		can_move[movingType][PT_NBHL] = 1;
		//nothing goes through stickmen
		can_move[movingType][PT_STKM] = 0;
		can_move[movingType][PT_STKM2] = 0;
		can_move[movingType][PT_FIGH] = 0;
		//INVS behaviour varies with pressure
		can_move[movingType][PT_INVIS] = 3;
		//stop CNCT from being displaced by other particles
		can_move[movingType][PT_CNCT] = 0;
		//VOID and PVOD behaviour varies with powered state and ctype
		can_move[movingType][PT_PVOD] = 3;
		can_move[movingType][PT_VOID] = 3;
		//nothing moves through EMBR (not sure why, but it's killed when it touches anything)
		can_move[movingType][PT_EMBR] = 0;
		can_move[PT_EMBR][movingType] = 0;
		//Energy particles move through VIBR and BVBR, so it can absorb them
		if (elements[movingType].Properties & TYPE_ENERGY)
		{
			can_move[movingType][PT_VIBR] = 1;
			can_move[movingType][PT_BVBR] = 1;
		}
	}
	//a list of lots of things PHOT can move through
	// TODO: replace with property
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		if (destinationType == PT_GLAS || destinationType == PT_PHOT || destinationType == PT_FILT || destinationType == PT_INVIS
		 || destinationType == PT_CLNE || destinationType == PT_PCLN || destinationType == PT_BCLN || destinationType == PT_PBCN
		 || destinationType == PT_WATR || destinationType == PT_DSTW || destinationType == PT_SLTW || destinationType == PT_GLOW
		 || destinationType == PT_ISOZ || destinationType == PT_ISZS || destinationType == PT_QRTZ || destinationType == PT_PQRT
		 || destinationType == PT_H2)
			can_move[PT_PHOT][destinationType] = 2;
		if (destinationType != PT_DMND && destinationType != PT_INSL && destinationType != PT_VOID && destinationType != PT_PVOD && destinationType != PT_VIBR)
			can_move[PT_PROT][destinationType] = 2;
	}

	//other special cases that weren't covered above
	can_move[PT_DEST][PT_DMND] = 0;
	can_move[PT_DEST][PT_CLNE] = 0;
	can_move[PT_DEST][PT_PCLN] = 0;
	can_move[PT_DEST][PT_BCLN] = 0;
	can_move[PT_DEST][PT_PBCN] = 0;

	can_move[PT_NEUT][PT_INVIS] = 2;
	can_move[PT_ELEC][PT_LCRY] = 2;
	can_move[PT_ELEC][PT_EXOT] = 2;
	can_move[PT_PHOT][PT_LCRY] = 3; //varies according to LCRY life

	can_move[PT_PHOT][PT_BIZR] = 2;
	can_move[PT_ELEC][PT_BIZR] = 2;
	can_move[PT_PHOT][PT_BIZRG] = 2;
	can_move[PT_ELEC][PT_BIZRG] = 2;
	can_move[PT_PHOT][PT_BIZRS] = 2;
	can_move[PT_ELEC][PT_BIZRS] = 2;
	can_move[PT_BIZR][PT_FILT] = 2;
	can_move[PT_BIZRG][PT_FILT] = 2;

	can_move[PT_ANAR][PT_WHOL] = 1; //WHOL eats ANAR
	can_move[PT_ANAR][PT_NWHL] = 1;
	can_move[PT_ELEC][PT_DEUT] = 1;
	can_move[PT_THDR][PT_THDR] = 2;
	can_move[PT_EMBR][PT_EMBR] = 2;
	can_move[PT_TRON][PT_SWCH] = 3;
}

/*
   RETURN-value explanation
1 = Swap
0 = No move/Bounce
2 = Both particles occupy the same space.
 */
int Simulation::eval_move(int pt, int nx, int ny, unsigned *rr)
{
	unsigned r;
	int result;

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 0;

	r = pmap[ny][nx];
	if (r)
		r = (r&~0xFF) | parts[r>>8].type;
	if (rr)
		*rr = r;
	if (pt>=PT_NUM || (r&0xFF)>=PT_NUM)
		return 0;
	result = can_move[pt][r&0xFF];
	if (result==3)
	{
		if ((pt==PT_PHOT || pt==PT_ELEC) && (r&0xFF)==PT_LCRY)
			result = (parts[r>>8].life > 5)? 2 : 0;
		if ((r&0xFF)==PT_INVIS)
		{
			if (pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f) result = 2;
			else result = 0;
		}
		else if ((r&0xFF)==PT_PVOD)
		{
			if (parts[r>>8].life == 10)
			{
				if(!parts[r>>8].ctype || (parts[r>>8].ctype==pt)!=(parts[r>>8].tmp&1))
					result = 1;
				else
					result = 0;
			}
			else result = 0;
		}
		else if ((r&0xFF)==PT_VOID)
		{
			if(!parts[r>>8].ctype || (parts[r>>8].ctype==pt)!=(parts[r>>8].tmp&1))
				result = 1;
			else
				result = 0;
		}
		else if (pt == PT_TRON && (r&0xFF) == PT_SWCH)
		{
			if (parts[r>>8].life >= 10)
				return 2;
			else
				return 0;
		}
	}
	if (bmap[ny/CELL][nx/CELL])
	{
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWGAS && !(elements[pt].Properties&TYPE_GAS))// && elements[pt].Falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWENERGY && !(elements[pt].Properties&TYPE_ENERGY))// && elements[pt].Falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWLIQUID && elements[pt].Falldown!=2)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWSOLID && elements[pt].Falldown!=1)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWAIR || bmap[ny/CELL][nx/CELL]==WL_WALL || bmap[ny/CELL][nx/CELL]==WL_WALLELEC)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_EWALL && !emap[ny/CELL][nx/CELL])
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL] && !(elements[pt].Properties&TYPE_SOLID) && !(elements[r&0xFF].Properties&TYPE_SOLID))
			return 2;
	}
	return result;
}

int Simulation::try_move(int i, int x, int y, int nx, int ny)
{
	unsigned r, e;

	if (x==nx && y==ny)
		return 1;
	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 1;

	e = eval_move(parts[i].type, nx, ny, &r);

	/* half-silvered mirror */
	if (!e && parts[i].type==PT_PHOT &&
	        (((r&0xFF)==PT_BMTL && rand()<RAND_MAX/2) ||
	         (pmap[y][x]&0xFF)==PT_BMTL))
		e = 2;

	if (!e) //if no movement
	{
		if (!(elements[parts[i].type].Properties & TYPE_ENERGY))
			return 0;
		if (!legacy_enable && parts[i].type==PT_PHOT && r)//PHOT heat conduction
		{
			if ((r & 0xFF) == PT_COAL || (r & 0xFF) == PT_BCOL)
				parts[r>>8].temp = parts[i].temp;

			if ((r & 0xFF) < PT_NUM && elements[r&0xFF].HeatConduct && ((r&0xFF)!=PT_HSWC||parts[r>>8].life==10) && (r&0xFF)!=PT_FILT)
				parts[i].temp = parts[r>>8].temp = restrict_flt((parts[r>>8].temp+parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
		}
		else if ((parts[i].type==PT_NEUT || parts[i].type==PT_ELEC) && ((r&0xFF)==PT_CLNE || (r&0xFF)==PT_PCLN || (r&0xFF)==PT_BCLN || (r&0xFF)==PT_PBCN)) {
			if (!parts[r>>8].ctype)
				parts[r>>8].ctype = parts[i].type;
		}
		if ((r&0xFF)==PT_PRTI && (elements[parts[i].type].Properties & TYPE_ENERGY))
		{
			int nnx, count;
			for (count=0; count<8; count++)
			{
				if (isign(x-nx)==isign(portal_rx[count]) && isign(y-ny)==isign(portal_ry[count]))
					break;
			}
			count = count%8;
			parts[r>>8].tmp = (int)((parts[r>>8].temp-73.15f)/100+1);
			if (parts[r>>8].tmp>=CHANNELS) parts[r>>8].tmp = CHANNELS-1;
			else if (parts[r>>8].tmp<0) parts[r>>8].tmp = 0;
			int portalx, portaly, blocker;
			portalx = parts[r>>8].x;
			portaly = parts[r>>8].y;
			blocker = BlockerWall[portaly/CELL][portalx/CELL];
			for ( nnx=0; nnx<80; nnx++)
				if (!portalp[blocker][parts[r>>8].tmp][count][nnx].type)
				{
					portalp[blocker][parts[r>>8].tmp][count][nnx] = parts[i];
					parts[i].type=PT_NONE;
					break;
				}
		}
		return 0;
	}

	if (e == 2) //if occupy same space
	{
		if (parts[i].type == PT_PHOT)
		{
			if ((r&0xFF) == PT_GLOW)
			{
				if (!parts[r>>8].life && rand() < RAND_MAX/30)
				{
					parts[r>>8].life = 120;
					create_gain_photon(i);
				}
			}
			else if ((r&0xFF) == PT_FILT)
				parts[i].ctype = Element_FILT::interactWavelengths(&parts[r>>8], parts[i].ctype);
			else if ((r&0xFF) == PT_INVIS)
			{
				if (pv[ny/CELL][nx/CELL]<=4.0f && pv[ny/CELL][nx/CELL]>=-4.0f)
				{
					part_change_type(i,x,y,PT_NEUT);
					parts[i].ctype = 0;
				}
			}
			else if ((r&0xFF)==PT_BIZR || (r&0xFF)==PT_BIZRG || (r&0xFF)==PT_BIZRS)
			{
				part_change_type(i, x, y, PT_ELEC);
				parts[i].ctype = 0;
			}
			else if ((r&0xFF) == PT_H2 && pv[y/CELL][x/CELL] < 45.0f && parts[i].temp < 3000)
			{
				part_change_type(i, x, y, PT_PROT);
				parts[i].ctype = 0;
				parts[i].tmp2 = 0x1;

				create_part(r>>8, x, y, PT_ELEC);
				return 1;
			}
		}
		else if (parts[i].type == PT_NEUT)
		{
			if ((r&0xFF) == PT_GLAS)
				if (rand() < RAND_MAX/10)
					create_cherenkov_photon(i);
		}
		else if (parts[i].type == PT_PROT)
		{
			if ((r&0xFF) == PT_INVIS)
				part_change_type(i, x, y, PT_NEUT);
		}
		else if ((parts[i].type==PT_BIZR || parts[i].type==PT_BIZRG))
		{
			if ((r&0xFF) == PT_FILT)
				parts[i].ctype = Element_FILT::interactWavelengths(&parts[r>>8], parts[i].ctype);
		}
		return 1;
	}
	//else e=1 , we are trying to swap the particles, return 0 no swap/move, 1 is still overlap/move, because the swap takes place later

	if ((r&0xFF)==PT_VOID || (r&0xFF)==PT_PVOD) //this is where void eats particles
	{
		//void ctype already checked in eval_move
		kill_part(i);
		return 0;
	}
	else if ((r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) //this is where blackhole eats particles
	{
		if (!legacy_enable)
		{
			parts[r>>8].temp = restrict_flt(parts[r>>8].temp+parts[i].temp/2, MIN_TEMP, MAX_TEMP);//3.0f;
		}
		kill_part(i);
		return 0;
	}
	else if (((r&0xFF)==PT_WHOL || (r&0xFF)==PT_NWHL)) //whitehole eats anar
	{
		if (parts[i].type == PT_ANAR)
		{
			if (!legacy_enable)
			{
				parts[r>>8].temp = restrict_flt(parts[r>>8].temp- (MAX_TEMP-parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
			}
			kill_part(i);
			return 0;
		}
	}
	else if ((r&0xFF)==PT_DEUT)
	{
		if (parts[i].type == PT_ELEC)
		{
			if(parts[r>>8].life < 6000)
				parts[r>>8].life += 1;
			parts[r>>8].temp = 0;
			kill_part(i);
			return 0;
		}
	}
	else if (((r&0xFF)==PT_VIBR || (r&0xFF)==PT_BVBR))
	{
		if ((elements[parts[i].type].Properties & TYPE_ENERGY))
		{
			parts[r>>8].tmp += 20;
			kill_part(i);
			return 0;
		}
	}

	if (parts[i].type == PT_NEUT)
	{
		if (elements[r & 0xFF].Properties & PROP_NEUTABSORB)
		{
			kill_part(i);
			return 0;
		}
	}
	else if (parts[i].type == PT_CNCT)
	{
		if (y<ny && (pmap[y+1][x]&0xFF) == PT_CNCT) //check below CNCT for another CNCT
			return 0;
	}
	else if(parts[i].type == PT_GBMB)
	{
		if (parts[i].life>0)
			return 0;
	}

	if ((bmap[y/CELL][x/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL]) && !(bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL]))
		return 0;

	e = r >> 8; //e is now the particle number at r (pmap[ny][nx])
	if (r)//the swap part, if we make it this far, swap
	{
		if (parts[i].type==PT_NEUT) {
			// target material is NEUTPENETRATE, meaning it gets moved around when neutron passes
			unsigned s = pmap[y][x];
			if (s && !(elements[s&0xFF].Properties&PROP_NEUTPENETRATE))
				return 1; // if the element currently underneath neutron isn't NEUTPENETRATE, don't move anything except the neutron
			// if nothing is currently underneath neutron, only move target particle
			if (s)
			{
				pmap[ny][nx] = (s&~(0xFF))|parts[s>>8].type;
				parts[s>>8].x = nx;
				parts[s>>8].y = ny;
			}
			else pmap[ny][nx] = 0;
			parts[e].x = x;
			parts[e].y = y;
			pmap[y][x] = (e<<8)|parts[e].type;
			return 1;
		}

		if ((pmap[ny][nx]>>8)==e) pmap[ny][nx] = 0;
		parts[e].x += x-nx;
		parts[e].y += y-ny;
		pmap[(int)(parts[e].y+0.5f)][(int)(parts[e].x+0.5f)] = (e<<8)|parts[e].type;
	}
	return 1;
}

// try to move particle, and if successful update pmap and parts[i].x,y
int Simulation::do_move(int i, int x, int y, float nxf, float nyf)
{
	int nx = (int)(nxf+0.5f), ny = (int)(nyf+0.5f), result;
	if (parts[i].type == PT_NONE)
		return 0;
	result = try_move(i, x, y, nx, ny);
	if (result)
	{
		int t = parts[i].type;
		parts[i].x = nxf;
		parts[i].y = nyf;
		if (ny!=y || nx!=x)
		{
			if ((pmap[y][x]>>8)==i) pmap[y][x] = 0;
			else if ((photons[y][x]>>8)==i) photons[y][x] = 0;
			if (nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)//kill_part if particle is out of bounds
			{
				kill_part(i);
				return -1;
			}
			if (elements[t].Properties & TYPE_ENERGY)
				photons[ny][nx] = t|(i<<8);
			else if (t)
				pmap[ny][nx] = t|(i<<8);
		}
	}
	return result;
}

int Simulation::pn_junction_sprk(int x, int y, int pt)
{
	unsigned r = pmap[y][x];
	if ((r & 0xFF) != pt)
		return 0;
	r >>= 8;
	if (parts[r].type != pt)
		return 0;
	if (parts[r].life != 0)
		return 0;

	parts[r].ctype = pt;
	part_change_type(r,x,y,PT_SPRK);
	parts[r].life = 4;
	return 1;
}

void Simulation::photoelectric_effect(int nx, int ny)//create sparks from PHOT when hitting PSCN and NSCN
{
	unsigned r = pmap[ny][nx];

	if ((r&0xFF) == PT_PSCN) {
		if ((pmap[ny][nx-1] & 0xFF) == PT_NSCN ||
		        (pmap[ny][nx+1] & 0xFF) == PT_NSCN ||
		        (pmap[ny-1][nx] & 0xFF) == PT_NSCN ||
		        (pmap[ny+1][nx] & 0xFF) == PT_NSCN)
			pn_junction_sprk(nx, ny, PT_PSCN);
	}
}

unsigned Simulation::direction_to_map(float dx, float dy, int t)
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

int Simulation::is_blocking(int t, int x, int y)
{
	if (t & REFRACT) {
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return 0;
		if ((pmap[y][x] & 0xFF) == PT_GLAS)
			return 1;
		return 0;
	}

	return !eval_move(t, x, y, NULL);
}

int Simulation::is_boundary(int pt, int x, int y)
{
	if (!is_blocking(pt,x,y))
		return 0;
	if (is_blocking(pt,x,y-1) && is_blocking(pt,x,y+1) && is_blocking(pt,x-1,y) && is_blocking(pt,x+1,y))
		return 0;
	return 1;
}

int Simulation::find_next_boundary(int pt, int *x, int *y, int dm, int *em)
{
	static int dx[8] = {1,1,0,-1,-1,-1,0,1};
	static int dy[8] = {0,1,1,1,0,-1,-1,-1};
	static int de[8] = {0x83,0x07,0x0E,0x1C,0x38,0x70,0xE0,0xC1};
	int i, ii, i0;

	if (*x <= 0 || *x >= XRES-1 || *y <= 0 || *y >= YRES-1)
		return 0;

	if (*em != -1) {
		i0 = *em;
		dm &= de[i0];
	} else
		i0 = 0;

	for (ii=0; ii<8; ii++) {
		i = (ii + i0) & 7;
		if ((dm & (1 << i)) && is_boundary(pt, *x+dx[i], *y+dy[i])) {
			*x += dx[i];
			*y += dy[i];
			*em = i;
			return 1;
		}
	}

	return 0;
}

int Simulation::get_normal(int pt, int x, int y, float dx, float dy, float *nx, float *ny)
{
	int ldm, rdm, lm, rm;
	int lx, ly, lv, rx, ry, rv;
	int i, j;
	float r, ex, ey;

	if (!dx && !dy)
		return 0;

	if (!is_boundary(pt, x, y))
		return 0;

	ldm = direction_to_map(-dy, dx, pt);
	rdm = direction_to_map(dy, -dx, pt);
	lx = rx = x;
	ly = ry = y;
	lv = rv = 1;
	lm = rm = -1;

	j = 0;
	for (i=0; i<SURF_RANGE; i++) {
		if (lv)
			lv = find_next_boundary(pt, &lx, &ly, ldm, &lm);
		if (rv)
			rv = find_next_boundary(pt, &rx, &ry, rdm, &rm);
		j += lv + rv;
		if (!lv && !rv)
			break;
	}

	if (j < NORMAL_MIN_EST)
		return 0;

	if ((lx == rx) && (ly == ry))
		return 0;

	ex = rx - lx;
	ey = ry - ly;
	r = 1.0f/hypot(ex, ey);
	*nx =  ey * r;
	*ny = -ex * r;

	return 1;
}

int Simulation::get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny)
{
	int x, y, i;

	dx /= NORMAL_FRAC;
	dy /= NORMAL_FRAC;

	for (i=0; i<NORMAL_INTERP; i++) {
		x = (int)(x0 + 0.5f);
		y = (int)(y0 + 0.5f);
		if (is_boundary(pt, x, y))
			break;
		x0 += dx;
		y0 += dy;
	}
	if (i >= NORMAL_INTERP)
		return 0;

	if (pt == PT_PHOT)
		photoelectric_effect(x, y);

	return get_normal(pt, x, y, dx, dy, nx, ny);
}

//For soap only
void Simulation::detach(int i)
{
	if ((parts[i].ctype&2) == 2)
	{
		if ((parts[parts[i].tmp].ctype&4) == 4)
			parts[parts[i].tmp].ctype ^= 4;
	}

	if ((parts[i].ctype&4) == 4)
	{
		if ((parts[parts[i].tmp2].ctype&2) == 2)
			parts[parts[i].tmp2].ctype ^= 2;
	}

	parts[i].ctype = 0;
}

void Simulation::kill_part(int i)//kills particle number i
{
	int x = (int)(parts[i].x+0.5f);
	int y = (int)(parts[i].y+0.5f);
	if (x>=0 && y>=0 && x<XRES && y<YRES) {
		if ((pmap[y][x]>>8)==i)
			pmap[y][x] = 0;
		else if ((photons[y][x]>>8)==i)
			photons[y][x] = 0;
	}

	if (parts[i].type == PT_NONE)
		return;

	if(parts[i].type > 0 && parts[i].type < PT_NUM && elementCount[parts[i].type])
		elementCount[parts[i].type]--;
	if (parts[i].type == PT_STKM)
	{
		player.spwn = 0;
	}
	else if (parts[i].type == PT_STKM2)
	{
		player2.spwn = 0;
	}
	else if (parts[i].type == PT_FIGH)
	{
		fighters[(unsigned char)parts[i].tmp].spwn = 0;
		fighcount--;
	}
	else if (parts[i].type == PT_SOAP)
	{
		detach(i);
	}

	parts[i].type = PT_NONE;
	parts[i].life = pfree;
	pfree = i;
}

void Simulation::part_change_type(int i, int x, int y, int t)//changes the type of particle number i, to t.  This also changes pmap at the same time.
{
	if (x<0 || y<0 || x>=XRES || y>=YRES || i>=NPART || t<0 || t>=PT_NUM || !parts[i].type)
		return;
	if (!elements[t].Enabled)
		t = PT_NONE;
	if (t == PT_NONE)
	{
		kill_part(i);
		return;
	}

	if (parts[i].type == PT_STKM)
		player.spwn = 0;
	else if (parts[i].type == PT_STKM2)
		player2.spwn = 0;
	else if (parts[i].type == PT_FIGH)
	{
		fighters[(unsigned char)parts[i].tmp].spwn = 0;
		fighcount--;
	}
	else if (parts[i].type == PT_SOAP)
		detach(i);

	parts[i].type = t;
	if (elements[t].Properties & TYPE_ENERGY)
	{
		photons[y][x] = t|(i<<8);
		if ((pmap[y][x]>>8)==i)
			pmap[y][x] = 0;
	}
	else
	{
		pmap[y][x] = t|(i<<8);
		if ((photons[y][x]>>8)==i)
			photons[y][x] = 0;
	}
}

//the function for creating a particle, use p=-1 for creating a new particle, -2 is from a brush, or a particle number to replace a particle.
//tv = Type (8 bits) + Var (24 bits), var is usually 0
int Simulation::create_part(int p, int x, int y, int tv)
{
	int i;

	int t = tv & 0xFF;
	int v = (tv >> 8) & 0xFFFFFF;

	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return -1;
	if (t>=0 && t<PT_NUM && !elements[t].Enabled)
		return -1;

	if (tv == SPC_AIR)
	{
		pv[y/CELL][x/CELL] += 0.03f;
		if (y+CELL<YRES)
			pv[y/CELL+1][x/CELL] += 0.03f;
		if (x+CELL<XRES)
		{
			pv[y/CELL][x/CELL+1] += 0.03f;
			if (y+CELL<YRES)
				pv[y/CELL+1][x/CELL+1] += 0.03f;
		}
		return -1;
	}

	if (t==PT_SPRK)
	{
		int type = pmap[y][x]&0xFF;
		int index = pmap[y][x]>>8;
		if(type == PT_WIRE)
		{
			parts[index].ctype = PT_DUST;
			return index;
		}
		if (p==-2 && ((elements[type].Properties & PROP_DRAWONCTYPE) || type==PT_CRAY))
		{
			parts[index].ctype = PT_SPRK;
			return index;
		}
		if (!(type == PT_INST || (elements[type].Properties&PROP_CONDUCTS)) || parts[index].life!=0)
			return -1;
		if (p == -2 && type == PT_INST)
		{
			FloodINST(x, y, PT_SPRK, PT_INST);
			return index;
		}
		parts[index].type = PT_SPRK;
		parts[index].life = 4;
		parts[index].ctype = type;
		pmap[y][x] = (pmap[y][x]&~0xFF) | PT_SPRK;
		if (parts[index].temp+10.0f < 673.0f && !legacy_enable && (type==PT_METL || type == PT_BMTL || type == PT_BRMT || type == PT_PSCN || type == PT_NSCN || type == PT_ETRD || type == PT_NBLE || type == PT_IRON))
			parts[index].temp = parts[index].temp+10.0f;
		return index;
	}
	if (t==PT_SPAWN&&elementCount[PT_SPAWN])
		return -1;
	if (t==PT_SPAWN2&&elementCount[PT_SPAWN2])
		return -1;
	if (p==-1)//creating from anything but brush
	{
		// If there is a particle, only allow creation if the new particle can occupy the same space as the existing particle
		// If there isn't a particle but there is a wall, check whether the new particle is allowed to be in it
		//   (not "!=2" for wall check because eval_move returns 1 for moving into empty space)
		// If there's no particle and no wall, assume creation is allowed
		if (pmap[y][x] ? (eval_move(t, x, y, NULL)!=2) : (bmap[y/CELL][x/CELL] && eval_move(t, x, y, NULL)==0))
		{
			if ((pmap[y][x]&0xFF)!=PT_SPAWN&&(pmap[y][x]&0xFF)!=PT_SPAWN2)
			{
				if (t!=PT_STKM&&t!=PT_STKM2&&t!=PT_FIGH)
				{
					return -1;
				}
			}
		}
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else if (p==-2)//creating from brush
	{
		if (pmap[y][x])
		{
			//If an element has the PROP_DRAWONCTYPE property, and the element being drawn to it does not have PROP_NOCTYPEDRAW (Also some special cases), set the element's ctype
			int drawOn = pmap[y][x]&0xFF;
			if (((elements[drawOn].Properties & PROP_DRAWONCTYPE) ||
				 (drawOn == PT_STOR && !(elements[t].Properties & TYPE_SOLID)) ||
				 (drawOn == PT_PCLN && t != PT_PSCN && t != PT_NSCN) ||
				 (drawOn == PT_PBCN && t != PT_PSCN && t != PT_NSCN))
				&& (!(elements[t].Properties & PROP_NOCTYPEDRAW)))
			{
				parts[pmap[y][x]>>8].ctype = t;
				if (t == PT_LIFE && v < NGOL && drawOn != PT_STOR)
					parts[pmap[y][x]>>8].tmp = v;
			}
			else if ((drawOn == PT_DTEC || (drawOn == PT_PSTN && t != PT_FRME)) && drawOn != t)
			{
				parts[pmap[y][x]>>8].ctype = t;
				if (drawOn == PT_DTEC && t==PT_LIFE && v<NGOL)
					parts[pmap[y][x]>>8].tmp = v;
			}
			else if (drawOn == PT_CRAY && drawOn != t && drawOn != PT_PSCN && drawOn != PT_INST && drawOn != PT_METL)
			{
				parts[pmap[y][x]>>8].ctype = t;
				if (t==PT_LIFE && v<NGOL)
					parts[pmap[y][x]>>8].tmp2 = v;
				parts[pmap[y][x]>>8].temp = elements[t].Temperature;
			}
			return -1;
		}
		else if (IsWallBlocking(x, y, t))
			return -1;
		if (photons[y][x] && (elements[t].Properties & TYPE_ENERGY))
			return -1;
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else if (p==-3)//skip pmap checks, e.g. for sing explosion
	{
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else
	{
		int oldX = (int)(parts[p].x+0.5f);
		int oldY = (int)(parts[p].y+0.5f);
		if ((pmap[oldY][oldX]>>8)==p)
			pmap[oldY][oldX] = 0;
		if ((photons[oldY][oldX]>>8)==p)
			photons[oldY][oldX] = 0;

		if (parts[p].type == PT_STKM)
		{
			player.spwn = 0;
		}
		else if (parts[p].type == PT_STKM2)
		{
			player2.spwn = 0;
		}
		else if (parts[p].type == PT_FIGH)
		{
			fighters[(unsigned char)parts[i].tmp].spwn = 0;
			fighcount--;
		}
		else if (parts[p].type == PT_SOAP)
		{
			detach(i);
		}
		i = p;
	}

	if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;

	parts[i].x = (float)x;
	parts[i].y = (float)y;
	parts[i].type = t;
	parts[i].vx = 0;
	parts[i].vy = 0;
	parts[i].life = 0;
	parts[i].ctype = 0;
	parts[i].temp = elements[t].Temperature;
	parts[i].tmp = 0;
	parts[i].tmp2 = 0;
	parts[i].dcolour = 0;
	parts[i].flags = 0;
	if (t == PT_GLAS || t == PT_QRTZ || t == PT_TUNG)
	{
		parts[i].pavg[0] = 0.0f;
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
	}
	else
	{
		parts[i].pavg[0] = 0.0f;
		parts[i].pavg[1] = 0.0f;
	}

	switch (t)
		{
			case PT_SOAP:
				parts[i].tmp = -1;
				parts[i].tmp2 = -1;
				break;
			case PT_ACID: case PT_CAUS:
				parts[i].life = 75;
				break;
			/*Testing
			case PT_WOOD:
				parts[i].life = 150;
				break;
			End Testing*/
			case PT_WARP:
				parts[i].life = rand()%95+70;
				break;
			case PT_FUSE:
				parts[i].life = 50;
				parts[i].tmp = 50;
				break;
			case PT_LIFE:
				if (v<NGOL)
				{
					parts[i].tmp = grule[v+1][9] - 1;
					parts[i].ctype = v;
				}
				break;
			case PT_DEUT:
				parts[i].life = 10;
				break;
			case PT_MERC:
				parts[i].tmp = 10;
				break;
			case PT_BRAY:
				parts[i].life = 30;
				break;
			case PT_GPMP: case PT_PUMP:
				parts[i].life = 10;
				break;
			case PT_SING:
				parts[i].life = rand()%50+60;
				break;
			case PT_QRTZ:
			case PT_PQRT:
				parts[i].tmp2 = (rand()%11);
				break;
			case PT_CLST:
				parts[i].tmp = (rand()%7);
				break;
			case PT_FSEP:
				parts[i].life = 50;
				break;
			case PT_COAL:
				parts[i].life = 110;
				parts[i].tmp = 50;
				break;
			case PT_IGNT:
				parts[i].life = 3;
				break;
			case PT_FRZW:
				parts[i].life = 100;
				break;
			case PT_PPIP:
			case PT_PIPE:
				parts[i].life = 60;
				break;
			case PT_BCOL:
				parts[i].life = 110;
				break;
			case PT_FIRE:
				parts[i].life = rand()%50+120;
				break;
			case PT_PLSM:
				parts[i].life = rand()%150+50;
				break;
			case PT_CFLM:
				parts[i].life = rand()%150+50;
				break;
			case PT_LAVA:
				parts[i].life = rand()%120+240;
				break;
			case PT_NBLE:
				parts[i].life = 0;
				break;
			case PT_ICEI:
				parts[i].ctype = PT_WATR;
				break;
			case PT_MORT:
				parts[i].vx = 2;
				break;
			case PT_EXOT:
				parts[i].life = 1000;
				parts[i].tmp = 244;
				break;
			case PT_EMBR:
				parts[i].life = 50;
				break;
			case PT_TESC:
				parts[i].tmp = v;
				if (parts[i].tmp > 300)
					parts[i].tmp=300;
				break;
			case PT_STKM:
				if (player.spwn==0)
				{
					parts[i].life = 100;
					Element_STKM::STKM_init_legs(this, &player, i);
					player.spwn = 1;
					player.rocketBoots = false;
				}
				else
				{
					parts[i].type=0;
					return -1;
				}
				create_part(-3,x,y,PT_SPAWN);
				break;
			case PT_STKM2:
				if (player2.spwn==0)
				{
					parts[i].life = 100;
					Element_STKM::STKM_init_legs(this, &player2, i);
					player2.spwn = 1;
					player2.rocketBoots = false;
				}
				else
				{
					parts[i].type=0;
					return -1;
				}
				create_part(-3,x,y,PT_SPAWN2);
				break;
			case PT_BIZR: case PT_BIZRG: case PT_BIZRS:
				parts[i].ctype = 0x47FFFF;
				break;
			case PT_DTEC:
			case PT_TSNS:
				parts[i].tmp2 = 2;
				break;
			case PT_VIRS:
			case PT_VRSS:
			case PT_VRSG:
				parts[i].pavg[1] = 250;
				break;
			case PT_FIGH:
			{
				unsigned char fcount = 0;
				while (fcount < 100 && fcount < (fighcount+1) && fighters[fcount].spwn==1) fcount++;
				if (fcount < 100 && fighters[fcount].spwn==0)
				{
					parts[i].life = 100;
					parts[i].tmp = fcount;
					Element_STKM::STKM_init_legs(this, &fighters[fcount], i);
					fighters[fcount].spwn = 1;
					fighters[fcount].elem = PT_DUST;
					fighters[fcount].rocketBoots = false;
					fighcount++;
					return i;
				}
				parts[i].type=0;
				return -1;
			}
			case PT_PHOT:
			{
				float a = (rand()%8) * 0.78540f;
				parts[i].life = 680;
				parts[i].ctype = 0x3FFFFFFF;
				parts[i].vx = 3.0f*cosf(a);
				parts[i].vy = 3.0f*sinf(a);
				break;
			}
			case PT_ELEC:
			{
				float a = (rand()%360)*3.14159f/180.0f;
				parts[i].life = 680;
				parts[i].vx = 2.0f*cosf(a);
				parts[i].vy = 2.0f*sinf(a);
				break;
			}
			case PT_NEUT:
			{
				float r = (rand()%128+128)/127.0f;
				float a = (rand()%360)*3.14159f/180.0f;
				parts[i].life = rand()%480+480;
				parts[i].vx = r*cosf(a);
				parts[i].vy = r*sinf(a);
				break;
			}
			case PT_PROT:
			{
				float a = (rand()%36)* 0.17453f;
				parts[i].life = 680;
				parts[i].vx = 2.0f*cosf(a);
				parts[i].vy = 2.0f*sinf(a);
				break;
			}
			case PT_TRON:
			{
				int randhue = rand()%360;
				int randomdir = rand()%4;
				parts[i].tmp = 1|(randomdir<<5)|(randhue<<7);//set as a head and a direction
				parts[i].tmp2 = 4;//tail
				parts[i].life = 5;
				break;
			}
			case PT_LIGH:
			{
				float gx, gy, gsize;
				if (p!=-2)
				{
					parts[i].life=30;
					parts[i].temp=parts[i].life*150.0f; // temperature of the lighting shows the power of the lighting
				}
				GetGravityField(x, y, 1.0f, 1.0f, gx, gy);
				gsize = gx*gx+gy*gy;
				if (gsize<0.0016f)
				{
					float angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
					gsize = sqrtf(gsize);
					// randomness in weak gravity fields (more randomness with weaker fields)
					gx += cosf(angle)*(0.04f-gsize);
					gy += sinf(angle)*(0.04f-gsize);
				}
				parts[i].tmp = (((int)(atan2f(-gy, gx)*(180.0f/M_PI)))+rand()%40-20+360)%360;
				parts[i].tmp2 = 4;
				break;
			}
			default:
				break;
		}
	//and finally set the pmap/photon maps to the newly created particle
	if (elements[t].Properties & TYPE_ENERGY)
		photons[y][x] = t|(i<<8);
	else if (t!=PT_STKM && t!=PT_STKM2 && t!=PT_FIGH)
		pmap[y][x] = t|(i<<8);

	//Fancy dust effects for powder types
	if((elements[t].Properties & TYPE_PART) && pretty_powder)
	{
		int colr, colg, colb, randa;
		randa = (rand()%30)-15;
		colr = (PIXR(elements[t].Colour)+sandcolour+(rand()%20)-10+randa);
		colg = (PIXG(elements[t].Colour)+sandcolour+(rand()%20)-10+randa);
		colb = (PIXB(elements[t].Colour)+sandcolour+(rand()%20)-10+randa);
		colr = colr>255 ? 255 : (colr<0 ? 0 : colr);
		colg = colg>255 ? 255 : (colg<0 ? 0 : colg);
		colb = colb>255 ? 255 : (colb<0 ? 0 : colb);
		parts[i].dcolour = 0xFF000000 | (colr<<16) | (colg<<8) | colb;
	}
	elementCount[t]++;
	return i;
}

void Simulation::GetGravityField(int x, int y, float particleGrav, float newtonGrav, float & pGravX, float & pGravY)
{
	pGravX = newtonGrav*gravx[(y/CELL)*(XRES/CELL)+(x/CELL)];
	pGravY = newtonGrav*gravy[(y/CELL)*(XRES/CELL)+(x/CELL)];
	switch (gravityMode)
	{
		default:
		case 0: //normal, vertical gravity
			pGravY += particleGrav;
			break;
		case 1: //no gravity
			break;
		case 2: //radial gravity
			if (x-XCNTR != 0 || y-YCNTR != 0)
			{
				float pGravMult = particleGrav/sqrtf((x-XCNTR)*(x-XCNTR) + (y-YCNTR)*(y-YCNTR));
				pGravX -= pGravMult * (float)(x - XCNTR);
				pGravY -= pGravMult * (float)(y - YCNTR);
			}
	}
}

void Simulation::create_gain_photon(int pp)//photons from PHOT going through GLOW
{
	float xx, yy;
	int i, lr, temp_bin, nx, ny;

	if (pfree == -1)
		return;
	i = pfree;

	lr = rand() % 2;

	if (lr) {
		xx = parts[pp].x - 0.3*parts[pp].vy;
		yy = parts[pp].y + 0.3*parts[pp].vx;
	} else {
		xx = parts[pp].x + 0.3*parts[pp].vy;
		yy = parts[pp].y - 0.3*parts[pp].vx;
	}

	nx = (int)(xx + 0.5f);
	ny = (int)(yy + 0.5f);

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return;

	if ((pmap[ny][nx] & 0xFF) != PT_GLOW)
		return;

	pfree = parts[i].life;
	if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;

	parts[i].type = PT_PHOT;
	parts[i].life = 680;
	parts[i].x = xx;
	parts[i].y = yy;
	parts[i].vx = parts[pp].vx;
	parts[i].vy = parts[pp].vy;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;
	parts[i].tmp = 0;
	parts[i].pavg[0] = parts[i].pavg[1] = 0.0f;
	photons[ny][nx] = PT_PHOT|(i<<8);

	temp_bin = (int)((parts[i].temp-273.0f)*0.25f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	parts[i].ctype = 0x1F << temp_bin;
}

void Simulation::create_cherenkov_photon(int pp)//photons from NEUT going through GLAS
{
	int i, lr, nx, ny;
	float r, eff_ior;

	if (pfree == -1)
		return;
	i = pfree;

	nx = (int)(parts[pp].x + 0.5f);
	ny = (int)(parts[pp].y + 0.5f);
	if ((pmap[ny][nx] & 0xFF) != PT_GLAS)
		return;

	if (hypotf(parts[pp].vx, parts[pp].vy) < 1.44f)
		return;

	pfree = parts[i].life;
	if (i>parts_lastActiveIndex) parts_lastActiveIndex = i;

	lr = rand() % 2;

	parts[i].type = PT_PHOT;
	parts[i].ctype = 0x00000F80;
	parts[i].life = 680;
	parts[i].x = parts[pp].x;
	parts[i].y = parts[pp].y;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;
	parts[i].tmp = 0;
	parts[i].pavg[0] = parts[i].pavg[1] = 0.0f;
	photons[ny][nx] = PT_PHOT|(i<<8);

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
	if (photons[y][x]) {
		i = photons[y][x];
	} else {
		i = pmap[y][x];
	}

	if (!i)
		return;
	kill_part(i>>8);
}

void Simulation::update_particles_i(int start, int inc)
{
	int i, j, x, y, t, nx, ny, r, surround_space, s, lt, rt, nt, nnx, nny, q, golnum, z, neighbors;
	float mv, dx, dy, nrx, nry, dp, ctemph, ctempl, gravtot;
	int fin_x, fin_y, clear_x, clear_y, stagnant;
	float fin_xf, fin_yf, clear_xf, clear_yf;
	float nn, ct1, ct2, swappage;
	float pt = R_TEMP;
	float c_heat = 0.0f;
	int h_count = 0;
	int surround[8];
	int surround_hconduct[8];
	unsigned int elem_properties;
	float pGravX, pGravY, pGravD;
	int excessive_stacking_found = 0;

	currentTick++;

	//if (sys_pause&&!framerender)//do nothing if paused
	//	return;

    if (force_stacking_check || (rand()%10)==0)
    {
    	force_stacking_check = 0;
    	excessive_stacking_found = 0;
    	for (y=0; y<YRES; y++)
    	{
    		for (x=0; x<XRES; x++)
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
    				else if (pmap_count[y][x]>1500 || (rand()%1600)<=(pmap_count[y][x]+100))
    				{
    					pmap_count[y][x] = pmap_count[y][x] + NPART;
    					excessive_stacking_found = 1;
    				}
    			}
    		}
    	}
    	if (excessive_stacking_found)
    	{
    		for (i=0; i<=parts_lastActiveIndex; i++)
    		{
    			if (parts[i].type)
    			{
    				t = parts[i].type;
    				x = (int)(parts[i].x+0.5f);
    				y = (int)(parts[i].y+0.5f);
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

	if (elementCount[PT_LOVE] > 0 || elementCount[PT_LOLZ] > 0) //LOVE and LOLZ element handling
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
				else if ((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&(parts[r>>8].type==PT_LOVE||parts[r>>8].type==PT_LOLZ))
					kill_part(r>>8);
				else if (parts[r>>8].type==PT_LOVE)
				{
					Element_LOVE::love[nx/9][ny/9] = 1;
				}
				else if (parts[r>>8].type==PT_LOLZ)
				{
					Element_LOLZ::lolz[nx/9][ny/9] = 1;
				}
			}
		}
		for (nx=9; nx<=XRES-18; nx++)
		{
			for (ny=9; ny<=YRES-7; ny++)
			{
				if (Element_LOVE::love[nx/9][ny/9]==1)
				{
					for ( nnx=0; nnx<9; nnx++)
						for ( nny=0; nny<9; nny++)
						{
							if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
							{
								rt=pmap[ny+nny][nx+nnx];
								if (!rt&&Element_LOVE::RuleTable[nnx][nny]==1)
									create_part(-1,nx+nnx,ny+nny,PT_LOVE);
								else if (!rt)
									continue;
								else if (parts[rt>>8].type==PT_LOVE&&Element_LOVE::RuleTable[nnx][nny]==0)
									kill_part(rt>>8);
							}
						}
				}
				Element_LOVE::love[nx/9][ny/9]=0;
				if (Element_LOLZ::lolz[nx/9][ny/9]==1)
				{
					for ( nnx=0; nnx<9; nnx++)
						for ( nny=0; nny<9; nny++)
						{
							if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
							{
								rt=pmap[ny+nny][nx+nnx];
								if (!rt&&Element_LOLZ::RuleTable[nny][nnx]==1)
									create_part(-1,nx+nnx,ny+nny,PT_LOLZ);
								else if (!rt)
									continue;
								else if (parts[rt>>8].type==PT_LOLZ&&Element_LOLZ::RuleTable[nny][nnx]==0)
									kill_part(rt>>8);

							}
						}
				}
				Element_LOLZ::lolz[nx/9][ny/9]=0;
			}
		}
	}

	//wire!
	if(elementCount[PT_WIRE] > 0)
	{
		for (nx=0; nx<XRES; nx++)
		{
			for (ny=0; ny<YRES; ny++)
		    {
			    r = pmap[ny][nx];
			    if (!r)
			        continue;
				if(parts[r>>8].type==PT_WIRE)
					parts[r>>8].tmp=parts[r>>8].ctype;
		    }
		}
	}

	if (Element_PPIP::ppip_changed)
	{
		for (i=0; i<=parts_lastActiveIndex; i++)
		{
			if (parts[i].type==PT_PPIP)
			{
				parts[i].tmp |= (parts[i].tmp&0xE0000000)>>3;
				parts[i].tmp &= ~0xE0000000;
			}
		}
		Element_PPIP::ppip_changed = 0;
	}

	//game of life!
	if (elementCount[PT_LIFE]>0&&++CGOL>=GSPEED)//GSPEED is frames per generation
	{
		CGOL=0;
		//TODO: maybe this should only loop through active particles
		for (ny=CELL; ny<YRES-CELL; ny++)
		{//go through every particle and set neighbor map
			for (nx=CELL; nx<XRES-CELL; nx++)
			{
				r = pmap[ny][nx];
				if (!r)
				{
					gol[ny][nx] = 0;
					continue;
				}
				if ((r&0xFF)==PT_LIFE)
				{
					golnum = parts[r>>8].ctype+1;
					if (golnum<=0 || golnum>NGOL) {
						kill_part(r>>8);
						continue;
					}
					gol[ny][nx] = golnum;
					if (parts[r>>8].tmp == grule[golnum][9]-1) {
						for ( nnx=-1; nnx<2; nnx++)
						{
							for ( nny=-1; nny<2; nny++)//it will count itself as its own neighbor, which is needed, but will have 1 extra for delete check
							{
								int adx = ((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL;
								int ady = ((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL;
								rt = pmap[ady][adx];
								if (!rt || (rt&0xFF)==PT_LIFE)
								{
									//the total neighbor count is in 0
									gol2[ady][adx][0] ++;
									//insert golnum into neighbor table
									for ( i=1; i<9; i++)
									{
										if (!gol2[ady][adx][i])
										{
											gol2[ady][adx][i] = (golnum<<4)+1;
											break;
										}
										else if((gol2[ady][adx][i]>>4)==golnum)
										{
											gol2[ady][adx][i]++;
											break;
										}
									}
								}
							}
						}
					} else {
						parts[r>>8].tmp --;
					}
				}
			}
		}
		for (ny=CELL; ny<YRES-CELL; ny++)
		{ //go through every particle again, but check neighbor map, then update particles
			for (nx=CELL; nx<XRES-CELL; nx++)
			{
				r = pmap[ny][nx];
				if (r && (r&0xFF)!=PT_LIFE)
					continue;
				neighbors = gol2[ny][nx][0];
				if (neighbors)
				{
					golnum = gol[ny][nx];
					if (!r)
					{
						//Find which type we can try and create
						int creategol = 0xFF;
						for ( i=1; i<9; i++)
						{
							if (!gol2[ny][nx][i]) break;
							golnum = (gol2[ny][nx][i]>>4);
							if (grule[golnum][neighbors]>=2 && (gol2[ny][nx][i]&0xF)>=(neighbors%2)+neighbors/2)
							{
								if (golnum<creategol) creategol=golnum;
							}
						}
						if (creategol<0xFF)
							create_part(-1, nx, ny, PT_LIFE|((creategol-1)<<8));
					}
					else if (grule[golnum][neighbors-1]==0 || grule[golnum][neighbors-1]==2)//subtract 1 because it counted itself
					{
						if (parts[r>>8].tmp==grule[golnum][9]-1)
							parts[r>>8].tmp --;
					}
					for ( z = 0; z<9; z++)
						gol2[ny][nx][z] = 0;//this improves performance A LOT compared to the memset, i was getting ~23 more fps with this.
				}
				//we still need to kill things with 0 neighbors (higher state life)
				if (r && parts[r>>8].tmp<=0)
						kill_part(r>>8);
			}
		}
		//memset(gol2, 0, sizeof(gol2));
	}
	if (ISWIRE>0)//wifi channel reseting
	{
		for ( q = 0; q<(int)(MAX_TEMP-73.15f)/100+2; q++)
		{
			for ( i = 0; i < BLOCKER_CHANNELS; i++)
			{
				wireless[i][q] = (wireless[i][q] << 1) & 2;
			}
		}
		ISWIRE--;
	}

	elementRecount |= !(currentTick%180);
	if(elementRecount)
	{
		std::fill(elementCount, elementCount+PT_NUM, 0);
	}

	for (i=0; i<=parts_lastActiveIndex; i++)
		if (parts[i].type)
		{
			t = parts[i].type;
			if (t<0 || t>=PT_NUM || !elements[t].Enabled)
			{
				kill_part(i);
				continue;
			}

			if(elementRecount)
				elementCount[t]++;

			elem_properties = elements[t].Properties;
			if (parts[i].life>0 && (elem_properties&PROP_LIFE_DEC))
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
			else if (parts[i].life<=0 && (elem_properties&PROP_LIFE_KILL))
			{
				// kill if no life
				kill_part(i);
				continue;
			}

			if (parts[i].type == PT_SPAWN && !player.spwn)
				create_part(-1, parts[i].x, parts[i].y, PT_STKM);
			else if (parts[i].type == PT_SPAWN2 && !player2.spwn)
				create_part(-1, parts[i].x, parts[i].y, PT_STKM2);
		}

	//the main particle loop function, goes over all particles.
	for (i=0; i<=parts_lastActiveIndex; i++)
		if (parts[i].type)
		{
			t = parts[i].type;

			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);

			//this kills any particle out of the screen, or in a wall where it isn't supposed to go
			if (x<CELL || y<CELL || x>=XRES-CELL || y>=YRES-CELL ||
			        (bmap[y/CELL][x/CELL] &&
			         (bmap[y/CELL][x/CELL]==WL_WALL ||
			          bmap[y/CELL][x/CELL]==WL_WALLELEC ||
			          bmap[y/CELL][x/CELL]==WL_ALLOWAIR ||
			          (bmap[y/CELL][x/CELL]==WL_DESTROYALL) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWLIQUID && elements[t].Falldown!=2) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWSOLID && elements[t].Falldown!=1) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWGAS && !(elements[t].Properties&TYPE_GAS)) || //&& elements[t].Falldown!=0 && parts[i].type!=PT_FIRE && parts[i].type!=PT_SMKE && parts[i].type!=PT_CFLM) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWENERGY && !(elements[t].Properties&TYPE_ENERGY)) ||
					  (bmap[y/CELL][x/CELL]==WL_DETECT && (t==PT_METL || t==PT_SPRK)) ||
			          (bmap[y/CELL][x/CELL]==WL_EWALL && !emap[y/CELL][x/CELL])) && (t!=PT_STKM) && (t!=PT_STKM2) && (t!=PT_FIGH)))
			{
				kill_part(i);
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
			if (elements[t].Gravity || !(elements[t].Properties & TYPE_SOLID))
			{
				//Gravity mode by Moach
				switch (gravityMode)
				{
				default:
				case 0:
					pGravX = 0.0f;
					pGravY = elements[t].Gravity;
					break;
				case 1:
					pGravX = pGravY = 0.0f;
					break;
				case 2:
					pGravD = 0.01f - hypotf((x - XCNTR), (y - YCNTR));
					pGravX = elements[t].Gravity * ((float)(x - XCNTR) / pGravD);
					pGravY = elements[t].Gravity * ((float)(y - YCNTR) / pGravD);
					break;
				}
				//Get some gravity from the gravity map
				if (t==PT_ANAR)
				{
					// perhaps we should have a ptypes variable for this
					pGravX -= gravx[(y/CELL)*(XRES/CELL)+(x/CELL)];
					pGravY -= gravy[(y/CELL)*(XRES/CELL)+(x/CELL)];
				}
				else if(t!=PT_STKM && t!=PT_STKM2 && t!=PT_FIGH && !(elements[t].Properties & TYPE_SOLID))
				{
					pGravX += gravx[(y/CELL)*(XRES/CELL)+(x/CELL)];
					pGravY += gravy[(y/CELL)*(XRES/CELL)+(x/CELL)];
				}
			}
			else
				pGravX = pGravY = 0;
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
#ifdef REALISTIC
				//The magic number controls diffusion speed
				parts[i].vx += 0.05*sqrtf(parts[i].temp)*elements[t].Diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
				parts[i].vy += 0.05*sqrtf(parts[i].temp)*elements[t].Diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
#else
				parts[i].vx += elements[t].Diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
				parts[i].vy += elements[t].Diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
#endif
			}

			j = surround_space = nt = 0;//if nt is greater than 1 after this, then there is a particle around the current particle, that is NOT the current particle's type, for water movement.
			for (nx=-1; nx<2; nx++)
				for (ny=-1; ny<2; ny++) {
					if (nx||ny) {
						surround[j] = r = pmap[y+ny][x+nx];
						j++;
						if (!(r&0xFF))
							surround_space++;//there is empty space
						if ((r&0xFF)!=t)
							nt++;//there is nothing or a different particle
					}
				}

			float gel_scale = 1.0f;
			if (t==PT_GEL)
				gel_scale = parts[i].tmp*2.55f;

			if (!legacy_enable)
			{
				if (y-2 >= 0 && y-2 < YRES && (elements[t].Properties&TYPE_LIQUID) && (t!=PT_GEL || gel_scale>(1+rand()%255))) {//some heat convection for liquids
					r = pmap[y-2][x];
					if (!(!r || parts[i].type != (r&0xFF))) {
						if (parts[i].temp>parts[r>>8].temp) {
							swappage = parts[i].temp;
							parts[i].temp = parts[r>>8].temp;
							parts[r>>8].temp = swappage;
						}
					}
				}

				//heat transfer code
				h_count = 0;
#ifdef REALISTIC
				if (t&&(t!=PT_HSWC||parts[i].life==10)&&(elements[t].HeatConduct*gel_scale))
#else
				if (t&&(t!=PT_HSWC||parts[i].life==10)&&(elements[t].HeatConduct*gel_scale)>(rand()%250))
#endif
				{
					float c_Cm = 0.0f;
					if (aheat_enable && !(elements[t].Properties&PROP_NOAMBHEAT))
					{
#ifdef REALISTIC
						c_heat = parts[i].temp*96.645/elements[t].HeatConduct*gel_scale*fabs(elements[t].Weight) + hv[y/CELL][x/CELL]*100*(pv[y/CELL][x/CELL]+273.15f)/256;
						c_Cm = 96.645/elements[t].HeatConduct*gel_scale*fabs(elements[t].Weight)  + 100*(pv[y/CELL][x/CELL]+273.15f)/256;
						pt = c_heat/c_Cm;
						pt = restrict_flt(pt, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
						parts[i].temp = pt;
						//Pressure increase from heat (temporary)
						pv[y/CELL][x/CELL] += (pt-hv[y/CELL][x/CELL])*0.004;
						hv[y/CELL][x/CELL] = pt;
#else
						c_heat = (hv[y/CELL][x/CELL]-parts[i].temp)*0.04;
						c_heat = restrict_flt(c_heat, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
						parts[i].temp += c_heat;
						hv[y/CELL][x/CELL] -= c_heat;
#endif
					}
					c_heat = 0.0f;
					c_Cm = 0.0f;
					for (j=0; j<8; j++)
					{
						surround_hconduct[j] = i;
						r = surround[j];
						if (!r)
							continue;
						rt = r&0xFF;
						if (rt&&elements[rt].HeatConduct&&(rt!=PT_HSWC||parts[r>>8].life==10)
						        &&(t!=PT_FILT||(rt!=PT_BRAY&&rt!=PT_BIZR&&rt!=PT_BIZRG))
						        &&(rt!=PT_FILT||(t!=PT_BRAY&&t!=PT_PHOT&&t!=PT_BIZR&&t!=PT_BIZRG))
						        &&(t!=PT_ELEC||rt!=PT_DEUT)
						        &&(t!=PT_DEUT||rt!=PT_ELEC))
						{
							surround_hconduct[j] = r>>8;
#ifdef REALISTIC
							if (rt==PT_GEL)
								gel_scale = parts[r>>8].tmp*2.55f;
							else gel_scale = 1.0f;

							c_heat += parts[r>>8].temp*96.645/elements[rt].HeatConduct*gel_scale*fabs(elements[rt].Weight);
							c_Cm += 96.645/elements[rt].HeatConduct*gel_scale*fabs(elements[rt].Weight);
#else
							c_heat += parts[r>>8].temp;
#endif
							h_count++;
						}
					}
#ifdef REALISTIC
					if (t==PT_GEL)
						gel_scale = parts[i].tmp*2.55f;
					else gel_scale = 1.0f;

					if (t == PT_PHOT)
						pt = (c_heat+parts[i].temp*96.645)/(c_Cm+96.645);
					else
						pt = (c_heat+parts[i].temp*96.645/elements[t].HeatConduct*gel_scale*fabs(elements[t].Weight))/(c_Cm+96.645/elements[t].HeatConduct*gel_scale*fabs(elements[t].Weight));

					c_heat += parts[i].temp*96.645/elements[t].HeatConduct*gel_scale*fabs(elements[t].Weight);
					c_Cm += 96.645/elements[t].HeatConduct*gel_scale*fabs(elements[t].Weight);
					parts[i].temp = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
#else
					pt = (c_heat+parts[i].temp)/(h_count+1);
					pt = parts[i].temp = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
					for (j=0; j<8; j++)
					{
						parts[surround_hconduct[j]].temp = pt;
					}
#endif

					ctemph = ctempl = pt;
					// change boiling point with pressure
					if ((elements[t].State==ST_LIQUID && elements[t].HighTemperatureTransition>-1 && elements[t].HighTemperatureTransition<PT_NUM && elements[elements[t].HighTemperatureTransition].State==ST_GAS)
					        || t==PT_LNTG || t==PT_SLTW)
						ctemph -= 2.0f*pv[y/CELL][x/CELL];
					else if ((elements[t].State==ST_GAS && elements[t].LowTemperatureTransition>-1 && elements[t].LowTemperatureTransition<PT_NUM && elements[elements[t].LowTemperatureTransition].State==ST_LIQUID)
					         || t==PT_WTRV)
						ctempl -= 2.0f*pv[y/CELL][x/CELL];
					s = 1;

					//A fix for ice with ctype = 0
					if ((t==PT_ICEI || t==PT_SNOW) && (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || parts[i].ctype==PT_ICEI || parts[i].ctype==PT_SNOW || !elements[parts[i].ctype].Enabled))
						parts[i].ctype = PT_WATR;

					if (elements[t].HighTemperatureTransition>-1 && ctemph>elements[t].HighTemperature)
					{
						// particle type change due to high temperature
#ifdef REALISTIC
						float dbt = ctempl - pt;
						if (elements[t].HighTemperatureTransition != PT_NUM)
						{
							if (platent[t] <= (c_heat - (elements[t].HighTemperature - dbt)*c_Cm))
							{
								pt = (c_heat - platent[t])/c_Cm;
								t = elements[t].HighTemperatureTransition;
							}
							else
							{
								parts[i].temp = restrict_flt(elements[t].HighTemperature - dbt, MIN_TEMP, MAX_TEMP);
								s = 0;
							}
						}
#else
						if (elements[t].HighTemperatureTransition != PT_NUM)
							t = elements[t].HighTemperatureTransition;
#endif
						else if (t == PT_ICEI || t == PT_SNOW)
						{
							if (parts[i].ctype > 0 && parts[i].ctype < PT_NUM && parts[i].ctype != t)
							{
								if (elements[parts[i].ctype].LowTemperatureTransition==t && pt<=elements[parts[i].ctype].LowTemperature)
									s = 0;
								else
								{
#ifdef REALISTIC
									//One ice table value for all it's kinds
									if (platent[t] <= (c_heat - (elements[parts[i].ctype].LowTemperature - dbt)*c_Cm))
									{
										pt = (c_heat - platent[t])/c_Cm;
										t = parts[i].ctype;
										parts[i].ctype = PT_NONE;
										parts[i].life = 0;
									}
									else
									{
										parts[i].temp = restrict_flt(elements[parts[i].ctype].LowTemperature - dbt, MIN_TEMP, MAX_TEMP);
										s = 0;
									}
#else
									t = parts[i].ctype;
									parts[i].ctype = PT_NONE;
									parts[i].life = 0;
#endif
								}
							}
							else
								s = 0;
						}
						else if (t == PT_SLTW)
						{
#ifdef REALISTIC
							if (platent[t] <= (c_heat - (elements[t].HighTemperature - dbt)*c_Cm))
							{
								pt = (c_heat - platent[t])/c_Cm;

								if (rand()%4==0) t = PT_SALT;
								else t = PT_WTRV;
							}
							else
							{
								parts[i].temp = restrict_flt(elements[t].HighTemperature - dbt, MIN_TEMP, MAX_TEMP);
								s = 0;
							}
#else
							if (rand()%4 == 0)
								t = PT_SALT;
							else
								t = PT_WTRV;
#endif
						}
						else if (t == PT_BRMT)
						{
							if (parts[i].ctype == PT_TUNG)
							{
								if (ctemph <= 3695.0)
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
						else
							s = 0;
					}
					else if (elements[t].LowTemperatureTransition > -1 && ctempl<elements[t].LowTemperature)
					{
						// particle type change due to low temperature
#ifdef REALISTIC
						float dbt = ctempl - pt;
						if (elements[t].LowTemperatureTransition != PT_NUM)
						{
							if (platent[elements[t].LowTemperatureTransition] >= (c_heat - (elements[t].LowTemperature - dbt)*c_Cm))
							{
								pt = (c_heat + platent[elements[t].LowTemperatureTransition])/c_Cm;
								t = elements[t].LowTemperatureTransition;
							}
							else
							{
								parts[i].temp = restrict_flt(elements[t].LowTemperature - dbt, MIN_TEMP, MAX_TEMP);
								s = 0;
							}
						}
#else
						if (elements[t].LowTemperatureTransition != PT_NUM)
							t = elements[t].LowTemperatureTransition;
#endif
						else if (t == PT_WTRV)
						{
							if (pt < 273.0f)
								t = PT_RIME;
							else
								t = PT_DSTW;
						}
						else if (t == PT_LAVA)
						{
							if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && parts[i].ctype!=PT_LAVA && parts[i].ctype!=PT_LAVA && elements[parts[i].ctype].Enabled)
							{
								if (parts[i].ctype==PT_THRM&&pt>=elements[PT_BMTL].HighTemperature)
									s = 0;
								else if ((parts[i].ctype==PT_VIBR || parts[i].ctype==PT_BVBR) && pt>=273.15f)
									s = 0;
								else if (parts[i].ctype==PT_TUNG)
								{
									if (pt>3695.0)
										s = 0;
								}
								else if (elements[parts[i].ctype].HighTemperatureTransition == PT_LAVA)
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
#ifdef REALISTIC
					pt = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
					for (j=0; j<8; j++)
					{
						parts[surround_hconduct[j]].temp = pt;
					}
#endif
					if (s) // particle type change occurred
					{
						if (t==PT_ICEI || t==PT_LAVA || t==PT_SNOW)
							parts[i].ctype = parts[i].type;
						if (!(t==PT_ICEI && parts[i].ctype==PT_FRZW))
							parts[i].life = 0;
						if (elements[t].State==ST_GAS && elements[parts[i].type].State!=ST_GAS)
							pv[y/CELL][x/CELL] += 0.50f;

						part_change_type(i,x,y,t);

						if (t==PT_FIRE || t==PT_PLSM || t==PT_CFLM)
							parts[i].life = rand()%50+120;
						if (t == PT_LAVA)
						{
							if (parts[i].ctype == PT_BRMT) parts[i].ctype = PT_BMTL;
							else if (parts[i].ctype == PT_SAND) parts[i].ctype = PT_GLAS;
							else if (parts[i].ctype == PT_BGLA) parts[i].ctype = PT_GLAS;
							else if (parts[i].ctype == PT_PQRT) parts[i].ctype = PT_QRTZ;
							parts[i].life = rand()%120+240;
						}
						if (t == PT_NONE)
						{
							kill_part(i);
							goto killed;
						}
					}

					pt = parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
					if (t == PT_LAVA)
					{
						parts[i].life = restrict_flt((parts[i].temp-700)/7, 0.0f, 400.0f);
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
				else parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
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
				nx = x % CELL;
				if (nx == 0)
					nx = x/CELL - 1;
				else if (nx == CELL-1)
					nx = x/CELL + 1;
				else
					nx = x/CELL;
				ny = y % CELL;
				if (ny == 0)
					ny = y/CELL - 1;
				else if (ny == CELL-1)
					ny = y/CELL + 1;
				else
					ny = y/CELL;
				if (nx>=0 && ny>=0 && nx<XRES/CELL && ny<YRES/CELL)
				{
					if (t!=PT_SPRK)
					{
						if (emap[ny][nx]==12 && !parts[i].life)
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
				parts[i].life = rand()%80+180;
				parts[i].temp = restrict_flt(elements[PT_FIRE].Temperature + (elements[t].Flammable/2), MIN_TEMP, MAX_TEMP);
				t = PT_FIRE;
				part_change_type(i,x,y,t);
				pv[y/CELL][x/CELL] += 0.25f * CFDS;
			}


			s = 1;
			gravtot = fabs(gravy[(y/CELL)*(XRES/CELL)+(x/CELL)])+fabs(gravx[(y/CELL)*(XRES/CELL)+(x/CELL)]);
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
			} else if (elements[t].LowPressureTransition>-1 && pv[y/CELL][x/CELL]<elements[t].LowPressure) {
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
				parts[i].life = 0;
				part_change_type(i,x,y,t);
				if (t==PT_FIRE)
					parts[i].life = rand()%50+120;
				if (t==PT_NONE)
				{
					kill_part(i);
					goto killed;
				}
			}

			//call the particle update function, if there is one
#if !defined(RENDERER) && defined(LUACONSOLE)
			if (elements[t].Update && lua_el_mode[t] != 2)
#else
			if (elements[t].Update)
#endif
			{
				if ((*(elements[t].Update))(this, i, x, y, surround_space, nt, parts, pmap))
					continue;
				else if (t==PT_WARP)
				{
					// Warp does some movement in its update func, update variables to avoid incorrect data in pmap
					x = (int)(parts[i].x+0.5f);
					y = (int)(parts[i].y+0.5f);
				}
			}
#if !defined(RENDERER) && defined(LUACONSOLE)
			if(lua_el_mode[t])
			{
				if(luacon_elementReplacement(this, i, x, y, surround_space, nt, parts, pmap))
					continue;
				// Need to update variables, in case they've been changed by Lua
				x = (int)(parts[i].x+0.5f);
				y = (int)(parts[i].y+0.5f);
			}
#endif

			if(legacy_enable)//if heat sim is off
				Element::legacyUpdate(this, i,x,y,surround_space,nt, parts, pmap);

killed:
			if (parts[i].type == PT_NONE)//if its dead, skip to next particle
				continue;

			if (!parts[i].vx&&!parts[i].vy)//if its not moving, skip to next particle, movement code it next
				continue;

			mv = fmaxf(fabsf(parts[i].vx), fabsf(parts[i].vy));
			if (mv < ISTP)
			{
				clear_x = x;
				clear_y = y;
				clear_xf = parts[i].x;
				clear_yf = parts[i].y;
				fin_xf = clear_xf + parts[i].vx;
				fin_yf = clear_yf + parts[i].vy;
				fin_x = (int)(fin_xf+0.5f);
				fin_y = (int)(fin_yf+0.5f);
			}
			else
			{
				// interpolate to see if there is anything in the way
				dx = parts[i].vx*ISTP/mv;
				dy = parts[i].vy*ISTP/mv;
				fin_xf = parts[i].x;
				fin_yf = parts[i].y;
				while (1)
				{
					mv -= ISTP;
					fin_xf += dx;
					fin_yf += dy;
					fin_x = (int)(fin_xf+0.5f);
					fin_y = (int)(fin_yf+0.5f);
					if (mv <= 0.0f)
					{
						// nothing found
						fin_xf = parts[i].x + parts[i].vx;
						fin_yf = parts[i].y + parts[i].vy;
						fin_x = (int)(fin_xf+0.5f);
						fin_y = (int)(fin_yf+0.5f);
						clear_xf = fin_xf-dx;
						clear_yf = fin_yf-dy;
						clear_x = (int)(clear_xf+0.5f);
						clear_y = (int)(clear_yf+0.5f);
						break;
					}
					if (fin_x<CELL || fin_y<CELL || fin_x>=XRES-CELL || fin_y>=YRES-CELL || pmap[fin_y][fin_x] || (bmap[fin_y/CELL][fin_x/CELL] && (bmap[fin_y/CELL][fin_x/CELL]==WL_DESTROYALL || !eval_move(t,fin_x,fin_y,NULL))))
					{
						// found an obstacle
						clear_xf = fin_xf-dx;
						clear_yf = fin_yf-dy;
						clear_x = (int)(clear_xf+0.5f);
						clear_y = (int)(clear_yf+0.5f);
						break;
					}
					if (bmap[fin_y/CELL][fin_x/CELL]==WL_DETECT && emap[fin_y/CELL][fin_x/CELL]<8)
						set_emap(fin_x/CELL, fin_y/CELL);
				}
			}

			stagnant = parts[i].flags & FLAG_STAGNANT;
			parts[i].flags &= ~FLAG_STAGNANT;

			if (t==PT_STKM || t==PT_STKM2 || t==PT_FIGH)
			{
				int nx, ny;
				//head movement, let head pass through anything
				parts[i].x += parts[i].vx;
				parts[i].y += parts[i].vy;
				nx = (int)((float)parts[i].x+0.5f);
				ny = (int)((float)parts[i].y+0.5f);
				if (ny!=y || nx!=x)
				{
					if ((pmap[y][x]>>8)==i) pmap[y][x] = 0;
					else if ((photons[y][x]>>8)==i) photons[y][x] = 0;
					if (nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)
					{
						kill_part(i);
						continue;
					}
					if (elements[t].Properties & TYPE_ENERGY)
						photons[ny][nx] = t|(i<<8);
					else if (t)
						pmap[ny][nx] = t|(i<<8);
				}
			}
			else if (elements[t].Properties & TYPE_ENERGY)
			{
				if (t == PT_PHOT) {
					if (parts[i].flags&FLAG_SKIPMOVE)
					{
						parts[i].flags &= ~FLAG_SKIPMOVE;
						continue;
					}

					rt = pmap[fin_y][fin_x] & 0xFF;
					lt = pmap[y][x] & 0xFF;

					r = eval_move(PT_PHOT, fin_x, fin_y, NULL);
					if (((rt==PT_GLAS && lt!=PT_GLAS) || (rt!=PT_GLAS && lt==PT_GLAS)) && r) {
						if (!get_normal_interp(REFRACT|t, parts[i].x, parts[i].y, parts[i].vx, parts[i].vy, &nrx, &nry)) {
							kill_part(i);
							continue;
						}

						r = get_wavelength_bin(&parts[i].ctype);
						if (r == -1) {
							kill_part(i);
							continue;
						}
						nn = GLASS_IOR - GLASS_DISP*(r-15)/15.0f;
						nn *= nn;
						nrx = -nrx;
						nry = -nry;
						if (rt==PT_GLAS && lt!=PT_GLAS)
							nn = 1.0f/nn;
						ct1 = parts[i].vx*nrx + parts[i].vy*nry;
						ct2 = 1.0f - (nn*nn)*(1.0f-(ct1*ct1));
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
					if (t==PT_NEUT && 100>(rand()%1000))
					{
						kill_part(i);
						continue;
					}
					r = pmap[fin_y][fin_x];

					if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && !(parts[r>>8].tmp&0xFF))
					{
						parts[r>>8].tmp =  (parts[r>>8].tmp&~0xFF) | parts[i].type;
						parts[r>>8].temp = parts[i].temp;
						parts[r>>8].tmp2 = parts[i].life;
						parts[r>>8].pavg[0] = parts[i].tmp;
						parts[r>>8].pavg[1] = parts[i].ctype;
						kill_part(i);
						continue;
					}

					// this should be replaced with a particle type attribute ("photwl" or something)
					if ((r & 0xFF) == PT_PSCN) parts[i].ctype  = 0x00000000;
					else if ((r & 0xFF) == PT_NSCN) parts[i].ctype  = 0x00000000;
					else if ((r & 0xFF) == PT_SPRK) parts[i].ctype  = 0x00000000;
					else if ((r & 0xFF) == PT_COAL) parts[i].ctype  = 0x00000000;
					else if ((r & 0xFF) == PT_BCOL) parts[i].ctype  = 0x00000000;
					else if ((r & 0xFF) == PT_PLEX) parts[i].ctype &= 0x1F00003E;
					else if ((r & 0xFF) == PT_NITR) parts[i].ctype &= 0x0007C000;
					else if ((r & 0xFF) == PT_NBLE) parts[i].ctype &= 0x3FFF8000;
					else if ((r & 0xFF) == PT_LAVA) parts[i].ctype &= 0x3FF00000;
					else if ((r & 0xFF) == PT_ACID) parts[i].ctype &= 0x1FE001FE;
					else if ((r & 0xFF) == PT_DUST) parts[i].ctype &= 0x3FFFFFC0;
					else if ((r & 0xFF) == PT_SNOW) parts[i].ctype &= 0x03FFFFFF;
					else if ((r & 0xFF) == PT_GOO)  parts[i].ctype &= 0x3FFAAA00;
					else if ((r & 0xFF) == PT_PLNT) parts[i].ctype &= 0x0007C000;
					else if ((r & 0xFF) == PT_PLUT) parts[i].ctype &= 0x001FCE00;
					else if ((r & 0xFF) == PT_URAN) parts[i].ctype &= 0x003FC000;
					else if ((r & 0xFF) == PT_GOLD) parts[i].ctype &= 0x3C038100;

					if (get_normal_interp(t, parts[i].x, parts[i].y, parts[i].vx, parts[i].vy, &nrx, &nry)) {
						dp = nrx*parts[i].vx + nry*parts[i].vy;
						parts[i].vx -= 2.0f*dp*nrx;
						parts[i].vy -= 2.0f*dp*nry;
						// leave the actual movement until next frame so that reflection of fast particles and refraction happen correctly
					} else {
						if (t!=PT_NEUT)
							kill_part(i);
						continue;
					}
					if (!(parts[i].ctype&0x3FFFFFFF) && t == PT_PHOT) {
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
				if (water_equal_test && elements[t].Falldown == 2 && 1>= rand()%400)//checking stagnant is cool, but then it doesn't update when you change it later.
				{
					if (!flood_water(x,y,i,y, parts[i].flags&FLAG_WATEREQUAL))
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
						s = 1;
						r = (rand()%2)*2-1;
						if ((clear_x!=x || clear_y!=y || nt || surround_space) &&
							(fabsf(parts[i].vx)>0.01f || fabsf(parts[i].vy)>0.01f))
						{
							// allow diagonal movement if target position is blocked
							// but no point trying this if particle is stuck in a block of identical particles
							dx = parts[i].vx - parts[i].vy*r;
							dy = parts[i].vy + parts[i].vx*r;
							if (fabsf(dy)>fabsf(dx))
								mv = fabsf(dy);
							else
								mv = fabsf(dx);
							dx /= mv;
							dy /= mv;
							if (do_move(i, x, y, clear_xf+dx, clear_yf+dy))
							{
								parts[i].vx *= elements[t].Collision;
								parts[i].vy *= elements[t].Collision;
								goto movedone;
							}
							swappage = dx;
							dx = dy*r;
							dy = -swappage*r;
							if (do_move(i, x, y, clear_xf+dx, clear_yf+dy))
							{
								parts[i].vx *= elements[t].Collision;
								parts[i].vy *= elements[t].Collision;
								goto movedone;
							}
						}
						if (elements[t].Falldown>1 && !grav->ngrav_enable && gravityMode==0 && parts[i].vy>fabsf(parts[i].vx))
						{
							s = 0;
							// stagnant is true if FLAG_STAGNANT was set for this particle in previous frame
							if (!stagnant || nt) //nt is if there is an something else besides the current particle type, around the particle
								rt = 30;//slight less water lag, although it changes how it moves a lot
							else
								rt = 10;

							if (t==PT_GEL)
								rt = parts[i].tmp*0.20f+5.0f;

							for (j=clear_x+r; j>=0 && j>=clear_x-rt && j<clear_x+rt && j<XRES; j+=r)
							{
								if (((pmap[fin_y][j]&0xFF)!=t || bmap[fin_y/CELL][j/CELL])
									&& (s=do_move(i, x, y, (float)j, fin_yf)))
								{
									nx = (int)(parts[i].x+0.5f);
									ny = (int)(parts[i].y+0.5f);
									break;
								}
								if (fin_y!=clear_y && ((pmap[clear_y][j]&0xFF)!=t || bmap[clear_y/CELL][j/CELL])
									&& (s=do_move(i, x, y, (float)j, clear_yf)))
								{
									nx = (int)(parts[i].x+0.5f);
									ny = (int)(parts[i].y+0.5f);
									break;
								}
								if ((pmap[clear_y][j]&0xFF)!=t || (bmap[clear_y/CELL][j/CELL] && bmap[clear_y/CELL][j/CELL]!=WL_STREAM))
									break;
							}
							if (parts[i].vy>0)
								r = 1;
							else
								r = -1;
							if (s==1)
								for (j=ny+r; j>=0 && j<YRES && j>=ny-rt && j<ny+rt; j+=r)
								{
									if (((pmap[j][nx]&0xFF)!=t || bmap[j/CELL][nx/CELL]) && do_move(i, nx, ny, (float)nx, (float)j))
										break;
									if ((pmap[j][nx]&255)!=t || (bmap[j/CELL][nx/CELL] && bmap[j/CELL][nx/CELL]!=WL_STREAM))
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
							s = 0;
							// stagnant is true if FLAG_STAGNANT was set for this particle in previous frame
							if (!stagnant || nt) //nt is if there is an something else besides the current particle type, around the particle
								rt = 30;//slight less water lag, although it changes how it moves a lot
							else
								rt = 10;
							nxf = clear_xf;
							nyf = clear_yf;
							for (j=0;j<rt;j++)
							{
								switch (gravityMode)
								{
									default:
									case 0:
										pGravX = 0.0f;
										pGravY = ptGrav;
										break;
									case 1:
										pGravX = pGravY = 0.0f;
										break;
									case 2:
										pGravD = 0.01f - hypotf((nx - XCNTR), (ny - YCNTR));
										pGravX = ptGrav * ((float)(nx - XCNTR) / pGravD);
										pGravY = ptGrav * ((float)(ny - YCNTR) / pGravD);
										break;
								}
								pGravX += gravx[(ny/CELL)*(XRES/CELL)+(nx/CELL)];
								pGravY += gravy[(ny/CELL)*(XRES/CELL)+(nx/CELL)];
								if (fabsf(pGravY)>fabsf(pGravX))
									mv = fabsf(pGravY);
								else
									mv = fabsf(pGravX);
								if (mv<0.0001f) break;
								pGravX /= mv;
								pGravY /= mv;
								if (j)
								{
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
								nx = (int)(nxf+0.5f);
								ny = (int)(nyf+0.5f);
								if (nx<0 || ny<0 || nx>=XRES || ny >=YRES)
									break;
								if ((pmap[ny][nx]&0xFF)!=t || bmap[ny/CELL][nx/CELL])
								{
									s = do_move(i, x, y, nxf, nyf);
									if (s)
									{
										nx = (int)(parts[i].x+0.5f);
										ny = (int)(parts[i].y+0.5f);
										break;
									}
									if (bmap[ny/CELL][nx/CELL]!=WL_STREAM)
										break;
								}
							}
							if (s==1)
							{
								clear_x = nx;
								clear_y = ny;
								for (j=0;j<rt;j++)
								{
									switch (gravityMode)
									{
										default:
										case 0:
											pGravX = 0.0f;
											pGravY = ptGrav;
											break;
										case 1:
											pGravX = pGravY = 0.0f;
											break;
										case 2:
											pGravD = 0.01f - hypotf((nx - XCNTR), (ny - YCNTR));
											pGravX = ptGrav * ((float)(nx - XCNTR) / pGravD);
											pGravY = ptGrav * ((float)(ny - YCNTR) / pGravD);
											break;
									}
									pGravX += gravx[(ny/CELL)*(XRES/CELL)+(nx/CELL)];
									pGravY += gravy[(ny/CELL)*(XRES/CELL)+(nx/CELL)];
									if (fabsf(pGravY)>fabsf(pGravX))
										mv = fabsf(pGravY);
									else
										mv = fabsf(pGravX);
									if (mv<0.0001f) break;
									pGravX /= mv;
									pGravY /= mv;
									nxf += pGravX;
									nyf += pGravY;
									nx = (int)(nxf+0.5f);
									ny = (int)(nyf+0.5f);
									if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
										break;
									if ((pmap[ny][nx]&0xFF)!=t || bmap[ny/CELL][nx/CELL])
									{
										s = do_move(i, clear_x, clear_y, nxf, nyf);
										if (s || bmap[ny/CELL][nx/CELL]!=WL_STREAM)
											break;
									}
								}
							}
							else if (s==-1) {} // particle is out of bounds
							else if ((clear_x!=x||clear_y!=y) && do_move(i, x, y, clear_xf, clear_yf)) {}
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

int Simulation::GetParticleType(std::string type)
{
	int i = -1;
	char * txt = (char*)type.c_str();

	// alternative names for some elements
	if (strcasecmp(txt,"C4")==0) i = PT_PLEX;
	else if (strcasecmp(txt,"C5")==0) i = PT_C5;
	else if (strcasecmp(txt,"NONE")==0) i = PT_NONE;
	for (i=1; i<PT_NUM; i++) {
		if (strcasecmp(txt, elements[i].Name)==0 && strlen(elements[i].Name) && elements[i].Enabled)
		{
			return i;
		}
	}
	return -1;
}

void Simulation::update_particles()//doesn't update the particles themselves, but some other things
{
	int i, x, y, t;
	int lastPartUsed = 0;
	int lastPartUnused = -1;
#ifdef MT
	int pt = 0, pc = 0;
	pthread_t *InterThreads;
#endif

	if(!sys_pause||framerender)
	{
		air->update_air();

		if(aheat_enable)
			air->update_airh();

		if(grav->ngrav_enable)
		{
			grav->gravity_update_async();

			//Get updated buffer pointers for gravity
			gravx = grav->gravx;
			gravy = grav->gravy;
			gravp = grav->gravp;
			gravmap = grav->gravmap;
		}
		if(gravWallChanged)
		{
			grav->gravity_mask();
			gravWallChanged = false;
		}
		if(emp_decor>0)
			emp_decor -= emp_decor/25+2;
		if(emp_decor < 0)
			emp_decor = 0;
	}
	sandcolour = (int)(20.0f*sin((float)sandcolour_frame*(M_PI/180.0f)));
	sandcolour_frame = (sandcolour_frame++)%360;

	memset(pmap, 0, sizeof(pmap));
	memset(pmap_count, 0, sizeof(pmap_count));
	memset(photons, 0, sizeof(photons));
	NUM_PARTS = 0;
	for (i=0; i<=parts_lastActiveIndex; i++)//the particle loop that resets the pmap/photon maps every frame, to update them.
	{
		if (parts[i].type)
		{
			t = parts[i].type;
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=0 && y>=0 && x<XRES && y<YRES)
			{
				if (elements[t].Properties & TYPE_ENERGY)
					photons[y][x] = t|(i<<8);
				else
				{
					// Particles are sometimes allowed to go inside INVS and FILT
					// To make particles collide correctly when inside these elements, these elements must not overwrite an existing pmap entry from particles inside them
					if (!pmap[y][x] || (t!=PT_INVIS && t!= PT_FILT))
						pmap[y][x] = t|(i<<8);
					// (there are a few exceptions, including energy particles - currently no limit on stacking those)
					if (t!=PT_THDR && t!=PT_EMBR && t!=PT_FIGH && t!=PT_PLSM)
						pmap_count[y][x]++;
				}
			}
			lastPartUsed = i;
			NUM_PARTS ++;
		}
		else
		{
			if (lastPartUnused<0) pfree = i;
			else parts[lastPartUnused].life = i;
			lastPartUnused = i;
		}
	}
	if (lastPartUnused==-1)
	{
		if (parts_lastActiveIndex>=NPART-1) pfree = -1;
		else pfree = parts_lastActiveIndex+1;
	}
	else
	{
		if (parts_lastActiveIndex>=NPART-1) parts[lastPartUnused].life = -1;
		else parts[lastPartUnused].life = parts_lastActiveIndex+1;
	}
	parts_lastActiveIndex = lastPartUsed;
	if (!sys_pause||framerender)
	{
		for (y=0; y<YRES/CELL; y++)
		{
			for (x=0; x<XRES/CELL; x++)
			{
				if (emap[y][x])
					emap[y][x] --;
				air->bmap_blockair[y][x] = (bmap[y][x]==WL_WALL || bmap[y][x]==WL_WALLELEC || (bmap[y][x]==WL_EWALL && !emap[y][x]));
				air->bmap_blockairh[y][x] = (bmap[y][x]==WL_WALL || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_GRAV || (bmap[y][x]==WL_EWALL && !emap[y][x]));
			}
		}
	}

	if(!sys_pause||framerender)
		update_particles_i(0, 1);

	if(framerender)
		framerender--;
}

Simulation::~Simulation()
{
	delete[] platent;
	delete grav;
	delete air;
	for(int i = 0; i < tools.size(); i++)
		delete tools[i];
}

Simulation::Simulation():
	sys_pause(0),
	framerender(0),
	aheat_enable(0),
	legacy_enable(0),
	gravityMode(0),
	edgeMode(0),
	water_equal_test(0),
	pretty_powder(0),
	sandcolour_frame(0),
	emp_decor(0),
	force_stacking_check(0),
	ISWIRE(0),
	gravWallChanged(false),
	replaceModeSelected(0),
	replaceModeFlags(0)
{
    int tportal_rx[] = {-1, 0, 1, 1, 1, 0,-1,-1};
    int tportal_ry[] = {-1,-1,-1, 0, 1, 1, 1, 0};
    
    memcpy(portal_rx, tportal_rx, sizeof(tportal_rx));   
    memcpy(portal_ry, tportal_ry, sizeof(tportal_ry));

    currentTick = 0;
    std::fill(elementCount, elementCount+PT_NUM, 0);
    elementRecount = true;

	//Create and attach gravity simulation
	grav = new Gravity();
	//Give air sim references to our data
	grav->bmap = bmap;
	//Gravity sim gives us maps to use
	gravx = grav->gravx;
	gravy = grav->gravy;
	gravp = grav->gravp;
	gravmap = grav->gravmap;

	//Create and attach air simulation
	air = new Air(*this);
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

	int menuCount;
	menu_section * msectionsT = LoadMenus(menuCount);
	memcpy(msections, msectionsT, menuCount * sizeof(menu_section));
	free(msectionsT);

	int wallCount;
	wall_type * wtypesT = LoadWalls(wallCount);
	memcpy(wtypes, wtypesT, wallCount * sizeof(wall_type));
	free(wtypesT);

	platent = new unsigned[PT_NUM];
	int latentCount;
	unsigned int * platentT = LoadLatent(latentCount);
	memcpy(platent, platentT, latentCount * sizeof(unsigned int));
	free(platentT);

	std::vector<Element> elementList = GetElements();
	DEFAULT_PT_NUM = elementList.size();
	for(int i = 0; i < PT_NUM; i++)
	{
		if(i < elementList.size())
			elements[i] = elementList[i];
		else
			elements[i] = Element();
	}
	
	tools = GetTools();

	int golRulesCount;
	int * golRulesT = LoadGOLRules(golRulesCount);
	memcpy(grule, golRulesT, sizeof(int) * (golRulesCount*10));
	free(golRulesT);

	int golTypesCount;
	int * golTypesT = LoadGOLTypes(golTypesCount);
	memcpy(goltype, golTypesT, sizeof(int) * (golTypesCount));
	free(golTypesT);

	int golMenuCount;
	gol_menu * golMenuT = LoadGOLMenu(golMenuCount);
	memcpy(gmenu, golMenuT, sizeof(gol_menu) * golMenuCount);
	free(golMenuT);

	player.comm = 0;
	player2.comm = 0;

	init_can_move();
	clear_sim();

	grav->gravity_mask();
}
