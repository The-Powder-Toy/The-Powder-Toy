#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PSTN PT_PSTN 168
Element_PSTN::Element_PSTN()
{
	Identifier = "DEFAULT_PT_PSTN";
	Name = "PSTN";
	Colour = PIXPACK(0xAA9999);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
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
	Hardness = 0;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 0;
	Description = "Piston, extends and pushes particles";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
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

#define PISTON_INACTIVE		0x00
#define PISTON_RETRACT		0x01
#define PISTON_EXTEND		0x02
#define MAX_FRAME			0x0F
#define DEFAULT_LIMIT		0x1F
#define DEFAULT_ARM_LIMIT	0xFF

//#TPT-Directive ElementHeader Element_PSTN static int update(UPDATE_FUNC_ARGS)
int Element_PSTN::update(UPDATE_FUNC_ARGS)
 {
 	if(parts[i].life)
 		return 0;
 	int maxSize = parts[i].tmp ? parts[i].tmp : DEFAULT_LIMIT;
 	int armLimit = parts[i].tmp2 ? parts[i].tmp2 : DEFAULT_ARM_LIMIT;
 	int state = 0;
	int r, nxx, nyy, nxi, nyi, rx, ry;
	int directionX = 0, directionY = 0;
	if (state == PISTON_INACTIVE) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry) && (!rx || !ry))
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
				if (BOUNDS_CHECK && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_PSTN)
					{
						bool movedPiston = false;
						bool foundEnd = false;
						int pistonEndX, pistonEndY;
						int pistonCount = 0;
						int newSpace = 0;
						int armCount = 0;
						directionX = rx;
						directionY = ry;
						for (nxx = 0, nyy = 0, nxi = directionX, nyi = directionY; ; nyy += nyi, nxx += nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if((r&0xFF)==PT_PSTN) {
								if(parts[r>>8].life)
									armCount++;
								else if (armCount)
								{
									pistonEndX = x+nxi+nxx;
									pistonEndY = y+nyi+nyy;
									foundEnd = true;
									break;
								}
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
								if(armCount+pistonCount > armLimit)
									pistonCount = armLimit-armCount;
								if(pistonCount > 0) {
									newSpace = MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, false, parts[i].ctype, true);
									if(newSpace) {
										//Create new piston section
										for(int j = 0; j < newSpace; j++) {
											int nr = sim->create_part(-3, pistonEndX+(nxi*j), pistonEndY+(nyi*j), PT_PSTN);
											if (nr > -1) {
												parts[nr].life = 1;
												if (parts[i].dcolour)
												{
													int red = PIXR(parts[i].dcolour)&0xFF;
													int green = PIXG(parts[i].dcolour);
													int blue = PIXB(parts[i].dcolour);
													parts[nr].dcolour = 255<<24|PIXRGB(red>60?red-60:0, green>60?green-60:0, blue>60?blue-60:0);
												}
											}
										}
										movedPiston =  true;
									}
								}
							} else if(state == PISTON_RETRACT) {
								if(pistonCount > armCount)
									pistonCount = armCount;
								if(armCount) {
									MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, true, parts[i].ctype, true);
									movedPiston = true;
								}
							}
						}
						if (movedPiston)
							return 0;
					}
				}

	}
	return 0;
}

//#TPT-Directive ElementHeader Element_PSTN static int CanMoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block)
int Element_PSTN::CanMoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block)
{
	int posX, posY, r, spaces = 0, currentPos = 0;
	if (amount == 0)
		return 0;
	for(posX = stackX, posY = stackY; currentPos < maxSize + amount; posX += directionX, posY += directionY) {
		if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
			break;
		}
		r = sim->pmap[posY][posX];
		if (sim->IsWallBlocking(posX, posY, 0) || (block && (r&0xFF) == block))
			break;
		if(!r) {
			spaces++;
			tempParts[currentPos++] = -1;
			if(spaces >= amount)
				break;
		} else {
			if(currentPos < maxSize && !retract)
				tempParts[currentPos++] = r>>8;
			else
				return spaces;
		}
	}
	if (spaces)
		return currentPos;
	else
		return 0;
}

//#TPT-Directive ElementHeader Element_PSTN static int MoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block, bool sticky, int callDepth = 0)
int Element_PSTN::MoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block, bool sticky, int callDepth)
{
	bool foundParts = false;
	int posX, posY, r, spaces = 0, currentPos = 0;
	r = sim->pmap[stackY][stackX];
	if(!callDepth && (r&0xFF) == PT_FRME) {
		int newY = !!directionX, newX = !!directionY;
		int realDirectionX = retract?-directionX:directionX;
		int realDirectionY = retract?-directionY:directionY;
		int maxRight = MAX_FRAME, maxLeft = MAX_FRAME;

		//check if we can push all the FRME
		for(int c = retract; c < MAX_FRAME; c++) {
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0 && (sim->pmap[posY][posX]&0xFF) == PT_FRME) {
				int val = CanMoveStack(sim, posX+realDirectionX, posY+realDirectionY, realDirectionX, realDirectionY, maxSize, amount, retract, block);
				if(val < amount)
					amount = val;
			} else {
				maxRight = c;
				break;
			}
		}
		for(int c = 1; c < MAX_FRAME; c++) {
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0 && (sim->pmap[posY][posX]&0xFF) == PT_FRME) {
				int val = CanMoveStack(sim, posX+realDirectionX, posY+realDirectionY, realDirectionX, realDirectionY, maxSize, amount, retract, block);
				if(val < amount)
					amount = val;
			} else {
				maxLeft = c;
				break;
			}
		}

		//If the piston is pushing frame, iterate out from the centre to the edge and push everything resting on frame
		for(int c = 1; c < maxRight; c++) {
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			MoveStack(sim, posX, posY, directionX, directionY, maxSize, amount, retract, block, !sim->parts[sim->pmap[posY][posX]>>8].tmp, 1);
		}
		for(int c = 1; c < maxLeft; c++) {
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			MoveStack(sim, posX, posY, directionX, directionY, maxSize, amount, retract, block, !sim->parts[sim->pmap[posY][posX]>>8].tmp, 1);
		}

		//Remove arm section if retracting with FRME
		if (retract)
			for(int j = 1; j <= amount; j++)
				sim->kill_part(sim->pmap[stackY+(directionY*-j)][stackX+(directionX*-j)]>>8);
		return MoveStack(sim, stackX, stackY, directionX, directionY, maxSize, amount, retract, block, !sim->parts[sim->pmap[stackY][stackX]>>8].tmp, 1);
	}
	if(retract){
		//Remove arm section if retracting without FRME
		if (!callDepth)
			for(int j = 1; j <= amount; j++)
				sim->kill_part(sim->pmap[stackY+(directionY*-j)][stackX+(directionX*-j)]>>8);
		bool foundEnd = false;
		for(posX = stackX, posY = stackY; currentPos < maxSize; posX += directionX, posY += directionY) {
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
				break;
			}
			r = sim->pmap[posY][posX];
			if(!r || (r&0xFF) == block || (!sticky && (r&0xFF) != PT_FRME)) {
				break;
			} else {
				foundParts = true;
				tempParts[currentPos++] = r>>8;
			}
		}
		if(foundParts) {
			//Move particles
			for(int j = 0; j < currentPos; j++) {
				int jP = tempParts[j];
				sim->pmap[(int)(sim->parts[jP].y + 0.5f)][(int)(sim->parts[jP].x + 0.5f)] = 0;
				sim->parts[jP].x += (float)((-directionX)*amount);
				sim->parts[jP].y += (float)((-directionY)*amount);
				sim->pmap[(int)(sim->parts[jP].y + 0.5f)][(int)(sim->parts[jP].x + 0.5f)] = sim->parts[jP].type|(jP<<8);
			}
			return amount;
		}
		if(!foundParts && foundEnd)
			return amount;
	} else {
		currentPos = CanMoveStack(sim, stackX, stackY, directionX, directionY, maxSize, amount, retract, block);
		if(currentPos){
			//Move particles
			int possibleMovement = 0;
			for(int j = currentPos-1; j >= 0; j--) {
				int jP = tempParts[j];
				if(jP < 0) {
					possibleMovement++;
					continue;
				}
				if(!possibleMovement)
					continue;
				sim->pmap[(int)(sim->parts[jP].y + 0.5f)][(int)(sim->parts[jP].x + 0.5f)] = 0;
				sim->parts[jP].x += (float)(directionX*possibleMovement);
				sim->parts[jP].y += (float)(directionY*possibleMovement);
				sim->pmap[(int)(sim->parts[jP].y + 0.5f)][(int)(sim->parts[jP].x + 0.5f)] = sim->parts[jP].type|(jP<<8);
			}
			return possibleMovement;
		}
		if(!foundParts && spaces)
			return spaces;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_PSTN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PSTN::graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life)
	{
		*colr -= 60;
		*colg -= 60;
	}
	return 0;
}

Element_PSTN::~Element_PSTN() {}
