#include "common/tpt-minmax.h"
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

	Temperature = 283.15f;
	HeatConduct = 0;
	Description = "Piston, extends and pushes particles.";

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

//#TPT-Directive ElementHeader Element_PSTN struct StackData
struct Element_PSTN::StackData
{
	int pushed;
	int spaces;
	
	StackData(int pushed, int spaces):
		pushed(pushed),
		spaces(spaces)
	{
	}
};

//#TPT-Directive ElementHeader Element_PSTN static int tempParts[XRES]
int Element_PSTN::tempParts[XRES];

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
					if (TYP(r)==PT_SPRK && parts[ID(r)].life==3) {
						if(parts[ID(r)].ctype == PT_PSCN)
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
					if (TYP(r) == PT_PSTN)
					{
						bool movedPiston = false;
						bool foundEnd = false;
						int pistonEndX, pistonEndY;
						int pistonCount = -1;// number of PSTN particles minus 1
						int newSpace = 0;
						int armCount = 0;
						directionX = rx;
						directionY = ry;
						for (nxx = 0, nyy = 0, nxi = directionX, nyi = directionY; ; nyy += nyi, nxx += nxi) {
							if (!(x+nxx<XRES && y+nyy<YRES && x+nxx >= 0 && y+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyy][x+nxx];
							if(TYP(r)==PT_PSTN)
							{
								if(parts[ID(r)].life)
									armCount++;
								else if (armCount)
								{
									pistonEndX = x+nxx;
									pistonEndY = y+nyy;
									foundEnd = true;
									break;
								}
								else
								{
									pistonCount += floor((parts[ID(r)].temp-268.15)/10);// How many tens of degrees above 0 C, rounded to nearest ten degrees. Can be negative.
								}
							}
							else if (nxx==0 && nyy==0)
							{
								// compatibility with BAD THINGS: starting PSTN layered underneath other particles
								// (in v90, it started scanning from the neighbouring particle, so could not break out of loop at offset=(0,0))
								pistonCount += floor((parts[i].temp-268.15)/10);
								continue;
							}
							else
							{
								pistonEndX = x+nxx;
								pistonEndY = y+nyy;
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
													int colour=parts[i].dcolour;
													parts[nr].dcolour=(colour&0xFF000000)|std::max((colour&0xFF0000)-0x3C0000,0)|std::max((colour&0xFF00)-0x3C00,0)|std::max((colour&0xFF)-0x3C,0);
												}
											}
										}
										movedPiston =  true;
									}
								}
							} else if(state == PISTON_RETRACT) {
								if(pistonCount > armCount)
									pistonCount = armCount;
								if(armCount && pistonCount > 0) {
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

//#TPT-Directive ElementHeader Element_PSTN static StackData CanMoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block)
Element_PSTN::StackData Element_PSTN::CanMoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block)
{
	int posX, posY, r, spaces = 0, currentPos = 0;
	if (amount <= 0)
		return StackData(0, 0);
	for (posX = stackX, posY = stackY; currentPos < maxSize + amount && currentPos < XRES-1; posX += directionX, posY += directionY)
	{
		if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0))
			break;

		r = sim->pmap[posY][posX];
		if (sim->IsWallBlocking(posX, posY, 0) || (block && TYP(r) == block))
			return StackData(currentPos - spaces, spaces);
		if (!r)
		{
			spaces++;
			tempParts[currentPos++] = -1;
			if (spaces >= amount)
				break;
		}
		else
		{
			if (currentPos - spaces < maxSize && (!retract || (TYP(r) == PT_FRME && posX == stackX && posY == stackY)))
				tempParts[currentPos++] = ID(r);
			else
				return StackData(currentPos - spaces, spaces);
		}
	}
	return StackData(currentPos - spaces, spaces);
}

//#TPT-Directive ElementHeader Element_PSTN static int MoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block, bool sticky, int callDepth = 0)
int Element_PSTN::MoveStack(Simulation * sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, bool retract, int block, bool sticky, int callDepth)
{
	int posX, posY, r;
	r = sim->pmap[stackY][stackX];
	if(!callDepth && TYP(r) == PT_FRME) {
		int newY = !!directionX, newX = !!directionY;
		int realDirectionX = retract?-directionX:directionX;
		int realDirectionY = retract?-directionY:directionY;
		int maxRight = MAX_FRAME, maxLeft = MAX_FRAME;

		//check if we can push all the FRME
		for(int c = retract; c < MAX_FRAME; c++) {
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0 && TYP(sim->pmap[posY][posX]) == PT_FRME) {
				int spaces = CanMoveStack(sim, posX, posY, realDirectionX, realDirectionY, maxSize, amount, retract, block).spaces;
				if(spaces < amount)
					amount = spaces;
			} else {
				maxRight = c;
				break;
			}
		}
		for(int c = 1; c < MAX_FRAME; c++) {
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0 && TYP(sim->pmap[posY][posX]) == PT_FRME) {
				int spaces = CanMoveStack(sim, posX, posY, realDirectionX, realDirectionY, maxSize, amount, retract, block).spaces;
				if(spaces < amount)
					amount = spaces;
			} else {
				maxLeft = c;
				break;
			}
		}

		//If the piston is pushing frame, iterate out from the centre to the edge and push everything resting on frame
		for(int c = 1; c < maxRight; c++) {
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			MoveStack(sim, posX, posY, directionX, directionY, maxSize, amount, retract, block, !sim->parts[ID(sim->pmap[posY][posX])].tmp, 1);
		}
		for(int c = 1; c < maxLeft; c++) {
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			MoveStack(sim, posX, posY, directionX, directionY, maxSize, amount, retract, block, !sim->parts[ID(sim->pmap[posY][posX])].tmp, 1);
		}

		//Remove arm section if retracting with FRME
		if (retract)
			for(int j = 1; j <= amount; j++)
				sim->kill_part(ID(sim->pmap[stackY+(directionY*-j)][stackX+(directionX*-j)]));
		return MoveStack(sim, stackX, stackY, directionX, directionY, maxSize, amount, retract, block, !sim->parts[ID(sim->pmap[stackY][stackX])].tmp, 1);
	}
	if(retract){
		bool foundParts = false;
		//Remove arm section if retracting without FRME
		if (!callDepth)
			for(int j = 1; j <= amount; j++)
				sim->kill_part(ID(sim->pmap[stackY+(directionY*-j)][stackX+(directionX*-j)]));
		int currentPos = 0;
		for(posX = stackX, posY = stackY; currentPos < maxSize && currentPos < XRES-1; posX += directionX, posY += directionY) {
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
				break;
			}
			r = sim->pmap[posY][posX];
			if(!r || TYP(r) == block || (!sticky && TYP(r) != PT_FRME)) {
				break;
			} else {
				foundParts = true;
				tempParts[currentPos++] = ID(r);
			}
		}
		if(foundParts) {
			//Move particles
			for(int j = 0; j < currentPos; j++) {
				int jP = tempParts[j];
				int srcX = (int)(sim->parts[jP].x + 0.5f), srcY = (int)(sim->parts[jP].y + 0.5f);
				int destX = srcX-directionX*amount, destY = srcY-directionY*amount;
				sim->pmap[srcY][srcX] = 0;
				sim->parts[jP].x = destX;
				sim->parts[jP].y = destY;
				sim->pmap[destY][destX] = PMAP(jP, sim->parts[jP].type);
			}
			return amount;
		}
	} else {
		StackData stackData = CanMoveStack(sim, stackX, stackY, directionX, directionY, maxSize, amount, retract, block);
		int currentPos = stackData.pushed + stackData.spaces;
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
				int srcX = (int)(sim->parts[jP].x + 0.5f), srcY = (int)(sim->parts[jP].y + 0.5f);
				int destX = srcX+directionX*possibleMovement, destY = srcY+directionY*possibleMovement;
				sim->pmap[srcY][srcX] = 0;
				sim->parts[jP].x = destX;
				sim->parts[jP].y = destY;
				sim->pmap[destY][destX] = PMAP(jP, sim->parts[jP].type);
			}
			return possibleMovement;
		}
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
