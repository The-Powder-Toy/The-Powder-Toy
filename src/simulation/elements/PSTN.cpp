#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PSTN PT_PSTN 168
Element_PSTN::Element_PSTN()
{
    Identifier = "DEFAULT_PT_PSTN";
    Name = "PSTN";
    Colour = PIXPACK(0xBBFF00);
    MenuVisible = 1;
    MenuSection = SC_ELEC;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.90f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f +273.15f;
    HeatConduct = 0;
    Description = "Piston, extends and pushes particles";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_LIFE_DEC;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_PSTN::update;
    Graphics = &Element_PSTN::graphics;
    
}

//#TPT-Directive ElementHeader Element_PSTN static int tempParts[128];
int Element_PSTN::tempParts[128];

#define PISTON_INACTIVE	0x00
#define PISTON_RETRACT	0x01
#define PISTON_EXTEND	0x02

//#TPT-Directive ElementHeader Element_PSTN static int update(UPDATE_FUNC_ARGS)
int Element_PSTN::update(UPDATE_FUNC_ARGS)
 {
 	if(parts[i].ctype)
 		return 0;
 	int maxSize = parts[i].tmp ? parts[i].tmp : 15;
 	int state = parts[i].tmp2;
	int r, nxx, nyy, nxi, nyi, rx, ry;
	int directionX = 0, directionY = 0;
	if (parts[i].life==0 && state == PISTON_INACTIVE) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
						if(parts[r>>8].ctype == PT_PSCN)
							state = PISTON_EXTEND;
						else
							state = PISTON_RETRACT;
					}
				}
	}
	if(state == PISTON_EXTEND || state == PISTON_RETRACT) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_PSTN && parts[r>>8].tmp2 == PISTON_INACTIVE) {
						directionX = rx;
						directionY = ry;
						{
							bool foundEnd = false;
							int pistonEndX, pistonEndY;
							int pistonCount = 1;
							int newSpace = 0;
							int armCount = 0;
							for (nxx = 0, nyy = 0, nxi = directionX, nyi = directionY; pistonCount < maxSize; nyy += nyi, nxx += nxi) {
								if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
									break;
								}
								r = pmap[y+nyi+nyy][x+nxi+nxx];
								if((r&0xFF)==PT_PSTN) {
									if(parts[r>>8].ctype)
										armCount++;
									else
										pistonCount++;
								} else {
									pistonEndX = x+nxi+nxx;
									pistonEndY = y+nyi+nyy;
									foundEnd = true;
									break;
								}
							}
							if(foundEnd) {
								if(state == PISTON_EXTEND) {
									newSpace = MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, false);
									if(newSpace) {
										//Create new piston section
										for(int j = 0; j < newSpace; j++) {
											int nr = sim->create_part(-3, pistonEndX+(nxi*j), pistonEndY+(nyi*j), PT_PSTN);
											if (nr!=-1) {
												parts[nr].ctype = 1;
											}
										}
									}
								} else if(state == PISTON_RETRACT) {
									if(pistonCount > armCount)
										pistonCount = armCount;
									if(armCount) {
										//Remove arm section
										int lastPistonX = pistonEndX - nxi;	//Go back to the very last piston arm particle
										int lastPistonY = pistonEndY - nyi;
										for(int j = 0; j < pistonCount; j++) {
											sim->delete_part(lastPistonX+(nxi*-j), lastPistonY+(nyi*-j), 0);
										}
										MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, true);
										//newSpace = MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, true);
									}
								}
							}
						}

						break;
					}
				}

	}
	return 0;
}

//#TPT-Directive ElementHeader Element_PSTN static int MoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int size, int amount, bool retract)
int Element_PSTN::MoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int size, int amount, bool retract)
{
	bool foundEnd = false, foundParts = false;
	int posX, posY, r, spaces = 0, currentPos = 0;
	if(retract){
		for(posX = stackX, posY = stackY; currentPos < size; posX += directionX, posY += directionY) {
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
				break;
			}
			r = sim->pmap[posY][posX];
			if(!r) {
				spaces++;
				foundEnd = true;
				if(spaces >= amount)
					break;
			} else {
				foundParts = true;
				tempParts[currentPos++] = r>>8;
			}
		}
		if(amount > spaces)
			amount = spaces;
		if(foundParts && foundEnd) {
			//Move particles
			for(int j = 0; j < currentPos; j++) {
				int jP = tempParts[j];
				sim->parts[jP].x += (float)((-directionX)*amount);
				sim->parts[jP].y += (float)((-directionY)*amount);
				int nPx = (int)(sim->parts[jP].x + 0.5f);
				int nPy = (int)(sim->parts[jP].y + 0.5f);
				sim->pmap[nPy][nPx] = sim->parts[jP].type|(jP<<8);
			}
			return amount;
		}
		if(!foundParts && foundEnd)
			return amount;		
	} else {
		for(posX = stackX, posY = stackY; currentPos < size; posX += directionX, posY += directionY) {
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
				break;
			}
			r = sim->pmap[posY][posX];
			if(!r) {
				spaces++;
				foundEnd = true;
				if(spaces >= amount)
					break;
			} else {
				foundParts = true;
				tempParts[currentPos++] = r>>8;
			}
		}
		if(amount > spaces)
			amount = spaces;
		if(foundParts && foundEnd) {
			//Move particles
			for(int j = 0; j < currentPos; j++) {
				int jP = tempParts[j];
				sim->parts[jP].x += (float)(directionX*amount);
				sim->parts[jP].y += (float)(directionY*amount);
				int nPx = (int)(sim->parts[jP].x + 0.5f);
				int nPy = (int)(sim->parts[jP].y + 0.5f);
				sim->pmap[nPy][nPx] = sim->parts[jP].type|(jP<<8);
			}
			return amount;
		}
		if(!foundParts && foundEnd)
			return amount;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_PSTN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PSTN::graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->ctype)
	{
		*colr -= 60;
		*colg -= 60;
	}
	return 0;
}

Element_PSTN::~Element_PSTN() {}